/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Universities Space Research Association nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SimulatorScriptReader.hh"

#include "Agenda.hh"
#include "CommandResponseManager.hh"
#include "GenericResponse.hh"
#include "LineInStream.hh"
#include "PlexilSimResponseFactory.hh"
#include "Simulator.hh"

#include "CommandHandle.hh"
#include "Debug.hh"
#include "timeval-utils.hh"
#include "Value.hh"

#include <cctype>

#include <iomanip> // std::setw(), std::setfill()
#include <iostream>
#include <memory>

using namespace PLEXIL;

enum SimSymbolType
  {
   UNKNOWN_SYM_TYPE = 0,

   LOOKUP_SYM_TYPE,
   COMMAND_SYM_TYPE,
   
   MAX_SYM_TYPE
  };

struct SimSymbol
{
  std::string name;
  SimSymbolType symbolType;
  ValueType returnType;

  SimSymbol(std::string const &nm, SimSymbolType symTyp, ValueType retTyp)
    : name(nm), symbolType(symTyp), returnType(retTyp)
  {
  }

};

class SimulatorScriptReaderImpl : public SimulatorScriptReader
{
private:

  //
  // Member data
  //

  std::map<std::string, std::unique_ptr<SimSymbol> > m_symbolTable;
  ResponseManagerMap *m_map;
  Agenda *m_agenda;
  std::unique_ptr<ResponseFactory> m_factory;

public:
  SimulatorScriptReaderImpl(ResponseManagerMap *map,
                            Agenda *agenda,
                            ResponseFactory *factory)
    : SimulatorScriptReader(),
      m_symbolTable(),
      m_map(map),
      m_agenda(agenda),
      m_factory(factory)
  {
    debugMsg("SimulatorScriptReader", " constructor");
  }

  virtual ~SimulatorScriptReaderImpl() = default;

  virtual bool readScript(const std::string &fName, bool telemetry = false)
  {
    assertTrue_2(m_factory,
                 "SimulatorScriptReader: null factory");
    
    debugMsg("SimulatorScriptReader:readScript",
             " for " << fName << ", telemetry = "
             << (telemetry ? "true" : "false"));

    bool compatibilityMode = false;
    if (telemetry)
      compatibilityMode = true;

    LineInStream instream;
    if (!instream.open(fName)) {
      std::cerr << "Error: cannot open script file \"" << fName << "\"" << std::endl;
      return false;
    }

    while (!instream.eof()) {
      std::istream &linestream = instream.getLine();
      std::string firstWord;
      linestream >> firstWord;
      if (firstWord.empty() && linestream.eof()) {
        debugMsg("SimulatorScriptReader:readScript",
                 " linestream at EOF");
        break;
      }

      debugMsg("SimulatorScriptReader:readScript",
               " read " << firstWord);

      // Check for declarations and mode lines

      // Backward compatibility
      if (firstWord == "BEGIN_TELEMETRY") {
        telemetry = true;
        compatibilityMode = true;
      }
      // More backward compatibility
      else if (firstWord == "BEGIN_COMMANDS") {
        telemetry = false;
        compatibilityMode = true;
      }
      else if (compatibilityMode) {
        if (telemetry) {
          if (!m_factory->parseTelemetryReturn(m_agenda, instream, firstWord, REAL_TYPE))
            break;
        }
        else if (!m_factory->parseCommandReturn(ensureResponseMessageManager(firstWord),
                                                instream, firstWord, REAL_TYPE))
          break;
      }
      else if (UNKNOWN_TYPE != parseValueType(firstWord)) {
        // Is declaration
        SimSymbol *sym = parseDeclaration(instream, parseValueType(firstWord));
        if (!sym) {
          // Report location of error
          std::cerr << " While parsing " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << std::endl;
          return false;
        }
        if (m_symbolTable.find(sym->name) != m_symbolTable.end()) {
          // Report duplicate symbol and return
          std::cerr << "Error: file " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << ": symbol \"" << sym->name << "\" is already declared"
                    << std::endl;
          return false;
        }
        m_symbolTable.emplace(sym->name, std::unique_ptr<SimSymbol>(sym));
      }
      else if (firstWord == "Command") {
        // Is command declaration w/ no return value
        SimSymbol *sym = parseCommandDeclaration(instream, UNKNOWN_TYPE);
        if (!sym) {
          // Report location of error
          std::cerr << " While parsing " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << std::endl;
          return false;
        }
        if (m_symbolTable.find(sym->name) != m_symbolTable.end()) {
          // Report duplicate symbol and return
          std::cerr << "Error: file " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << ": symbol \"" << sym->name << "\" is already declared"
                    << std::endl;
          return false;
        }
        m_symbolTable.emplace(sym->name, std::unique_ptr<SimSymbol>(sym));
      }
      else if (m_symbolTable.empty() && !telemetry && !compatibilityMode) {
        // Presume this is first line of old-style command script
        debugMsg("SimulatorScriptReader:readScript",
                 " presuming old-style command script");
        compatibilityMode = true;
        if (!m_factory->parseCommandReturn(ensureResponseMessageManager(firstWord),
                                           instream, firstWord, REAL_TYPE))
          break;
      }
      else if (m_symbolTable.find(firstWord) != m_symbolTable.end()) {
        // This is a known symbol, parse according to symbol type
        SimSymbol *sym = m_symbolTable[firstWord].get();
        if (sym->symbolType == LOOKUP_SYM_TYPE) {
          if (!m_factory->parseTelemetryReturn(m_agenda, instream, firstWord, sym->returnType))
            break;
        }
        else if (!m_factory->parseCommandReturn(ensureResponseMessageManager(firstWord),
                                                instream, firstWord, sym->returnType))
          break;
      }
      else {
        std::cerr << "Error: file " << instream.getFileName()
                  << ", line " << instream.getLineCount()
                  << ": format error; don't know how to interpret \"" << firstWord << '"'
                  << std::endl;
        return false;
      }
    }
  
    instream.close();

    return true;
  }

private:

  // First word has already been parsed as a type
  SimSymbol *parseDeclaration(LineInStream &instream, ValueType returnType)
  {
    std::string word;
    instream.getLineStream() >> word;
    // Check for I/O error
    // TODO

    if (word == "Command")
      return parseCommandDeclaration(instream, returnType);
    if (word != "Lookup") {
      // Report parse error
      std::cerr << "Error: File " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ":\n found \"" << word << "\", expected Command or Lookup"
                << std::endl;
      return nullptr;
    }


    // Below this point is known to be a Lookup
    // Get name
    instream.getLineStream() >> word;
    // Check for I/O error
    // TODO

    debugMsg("SimulatorScriptReader:parseLookupDeclaration",
             ' ' << word);

    // Parse parameter declarations
    // NYI

    // Construct and return the symbol
    return new SimSymbol(word, LOOKUP_SYM_TYPE, returnType);
  }

  // When we know the declaration is a command
  // Word 'Command' has already been parsed;
  // if it was preceded by a type name, that is the return type.
  SimSymbol *parseCommandDeclaration(LineInStream &instream, ValueType returnType)
  {
    // Get name
    std::string word;
    instream.getLineStream() >> word;
    // Check for I/O error
    // TODO

    debugMsg("SimulatorScriptReader:parseCommandDeclaration",
             ' ' << word);

    // Parse parameter declarations
    // NYI

    // Construct and return the symbol
    return new SimSymbol(word, COMMAND_SYM_TYPE, returnType);
  }

  CommandResponseManager* 
  ensureResponseMessageManager(const std::string& name)
  {
    debugMsg("SimulatorScriptReader:ensureResponseMessageManager",
             " " << name);
    ResponseManagerMap::const_iterator it = m_map->find(name);
    if (it != m_map->end()) {
      debugMsg("SimulatorScriptReader:ensureResponseMessageManager",
               " " << name << " exists");
      return it->second.get();
    }

    CommandResponseManager* result = makeCommandResponseManager(name);
    m_map->emplace(name, std::unique_ptr<CommandResponseManager>(result));
    return result;
  }

};

SimulatorScriptReader *makeScriptReader(ResponseManagerMap *map,
                                        Agenda *agenda,
                                        ResponseFactory *factory)
{
  return new SimulatorScriptReaderImpl(map, agenda, factory);
}
