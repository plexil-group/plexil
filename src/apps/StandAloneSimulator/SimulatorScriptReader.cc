/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "Simulator.hh"

#include "CommandHandle.hh"
#include "Debug.hh"
#include "timeval-utils.hh"
#include "Value.hh"

#include <cctype>

#include <fstream>
#include <iomanip> // std::setw(), std::setfill()
#include <iostream>
#include <sstream>

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

#define MAX_LINE_LENGTH (1024)

// Helper class
class LineInStream {
public:

  LineInStream()
    : m_filename(),
      m_filestream(),
      m_linestream(),
      m_linecount(0),
      m_linebuf(MAX_LINE_LENGTH, '\0')
  {
  }

  ~LineInStream()
  {
  }

  // (Re)Open the stream with a new file
  // Returns true on success
  bool open(std::string const &fname)
  {
    close();
    m_linecount = 0;
    m_filestream.open(fname.c_str());
    if (m_filestream.fail()) {
      debugMsg("LineInStream:open", " for " << fname << " failed");
      return false;
    }
    debugMsg("LineInStream:open", ' ' << fname);
    m_filename = fname;
    return true;
  }

  void close()
  {
    if (m_filestream.is_open()) {
      m_filestream.close();
      m_filename.clear();
    }
  }

  std::istream &getLine()
  {
    if (!m_filestream.good() || m_filestream.eof()) {
      debugMsg("LineInStream:getLine", " at EOF or error");
      m_linebuf.clear();
    }

    bool ignoreLine = false;
    do {
      debugMsg("LineInStream:getLine", " not EOF");
      std::getline(m_filestream, m_linebuf);
      ++m_linecount;
      size_t firstNonWhitespace = m_linebuf.find_first_not_of(" \t\n\r");
      ignoreLine = (m_linebuf.size() == 0)
        || (std::string::npos == firstNonWhitespace)
        || !isalnum(m_linebuf[firstNonWhitespace]);
      condDebugMsg(ignoreLine,
                   "LineInStream:getLine",
                   " ignoring blank or comment line");
    }
    while (ignoreLine && m_filestream.good() && !m_filestream.eof());

    debugMsg("LineInStream:getLine",
             " line = \"" << m_linebuf << "\"");

    m_linestream.clear();
    m_linestream.str(m_linebuf);
    return m_linestream;
  }

  std::istringstream &getLineStream()
  {
    return m_linestream;
  }

  std::string const &getFileName() const
  {
    return m_filename;
  }

  unsigned int getLineCount() const
  {
    return m_linecount;
  }

  bool good() const
  {
    return m_filestream.good();
  }

  bool eof() const
  {
    return m_filestream.eof();
  }

private:
  std::string m_filename;
  std::ifstream m_filestream;
  std::istringstream m_linestream;
  unsigned int m_linecount;
  std::string m_linebuf;
};


class PlexilSimScriptReader : public SimulatorScriptReader
{
private:

  //
  // Member data
  //

  std::map<std::string, SimSymbol *> m_symbolTable;
  LineInStream m_instream;
  ResponseManagerMap *m_map;
  Agenda *m_agenda;

public:
  PlexilSimScriptReader(ResponseManagerMap *map,
                        Agenda *agenda)
    : SimulatorScriptReader(),
      m_symbolTable(),
      m_instream(),
      m_map(map),
      m_agenda(agenda)
  {
    debugMsg("SimulatorScriptReader:constructor", "");
  }

  virtual ~PlexilSimScriptReader()
  {
    // Clean up symbol table
    while (!m_symbolTable.empty()) {
      SimSymbol *entry = m_symbolTable.begin()->second;
      m_symbolTable.erase(m_symbolTable.begin());
      delete entry;
    }
  }

  virtual bool readScript(const std::string &fName, bool telemetry = false)
  {
    debugMsg("SimulatorScriptReader:readScript",
             " for " << fName << ", telemetry = "
             << (telemetry ? "true" : "false"));

    bool compatibilityMode = false;
    if (telemetry)
      compatibilityMode = true;

    if (!m_instream.open(fName)) {
      std::cerr << "Error: cannot open script file \"" << fName << "\"" << std::endl;
      return false;
    }

    while (!m_instream.eof()) {
      std::istream &linestream = m_instream.getLine();
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
          if (!parseTelemetryReturn(firstWord, NULL, compatibilityMode))
            break;
        }
        else if (!parseCommandReturn(firstWord, NULL, compatibilityMode))
          break;
      }
      else if (UNKNOWN_TYPE != parseValueType(firstWord)) {
        // Is declaration
        SimSymbol *sym = parseDeclaration(parseValueType(firstWord));
        if (!sym) {
          // Report location of error
          std::cerr << " While parsing " << m_instream.getFileName()
                    << ", line " << m_instream.getLineCount()
                    << std::endl;
          return false;
        }
        if (m_symbolTable.find(sym->name) != m_symbolTable.end()) {
          // Report duplicate symbol and return
          std::cerr << "Error: file " << m_instream.getFileName()
                    << ", line " << m_instream.getLineCount()
                    << ": symbol \"" << sym->name << "\" is already declared"
                    << std::endl;
          return false;
        }
        m_symbolTable[sym->name] = sym;
      }
      else if (firstWord == "Command") {
        // Is command declaration w/ no return value
        SimSymbol *sym = parseCommandDeclaration(UNKNOWN_TYPE);
        if (!sym) {
          // Report location of error
          std::cerr << " While parsing " << m_instream.getFileName()
                    << ", line " << m_instream.getLineCount()
                    << std::endl;
          return false;
        }
        if (m_symbolTable.find(sym->name) != m_symbolTable.end()) {
          // Report duplicate symbol and return
          std::cerr << "Error: file " << m_instream.getFileName()
                    << ", line " << m_instream.getLineCount()
                    << ": symbol \"" << sym->name << "\" is already declared"
                    << std::endl;
          return false;
        }
        m_symbolTable[sym->name] = sym;
      }
      else if (m_symbolTable.empty() && !telemetry && !compatibilityMode) {
        // Presume this is first line of old-style command script
        debugMsg("SimulatorScriptReader:readScript",
                 " presuming old-style command script");
        compatibilityMode = true;
        if (!parseCommandReturn(firstWord, NULL, compatibilityMode))
          break;
      }
      else if (m_symbolTable.find(firstWord) != m_symbolTable.end()) {
        // This is a known symbol, parse according to symbol type
        SimSymbol *sym = m_symbolTable[firstWord];
        if (sym->symbolType == LOOKUP_SYM_TYPE) {
          if (!parseTelemetryReturn(firstWord, sym))
            break;
        }
        else if (!parseCommandReturn(firstWord, sym))
          break;
      }
      else {
        std::cerr << "Error: file " << m_instream.getFileName()
                  << ", line " << m_instream.getLineCount()
                  << ": format error; don't know how to interpret \"" << firstWord << '"'
                  << std::endl;
        return false;
      }
    }
  
    m_instream.close();

    return true;
  }

private:

  // First word has already been parsed as a type
  SimSymbol *parseDeclaration(ValueType returnType)
  {
    std::string word;
    m_instream.getLineStream() >> word;
    // Check for I/O error
    // TODO

    if (word == "Command")
      return parseCommandDeclaration(returnType);
    if (word != "Lookup") {
      // Report parse error
      std::cerr << "Error: File " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ":\n found \"" << word << "\", expected Command or Lookup"
                << std::endl;
      return NULL;
    }


    // Below this point is known to be a Lookup
    // Get name
    m_instream.getLineStream() >> word;
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
  SimSymbol *parseCommandDeclaration(ValueType returnType)
  {
    // Get name
    std::string word;
    m_instream.getLineStream() >> word;
    // Check for I/O error
    // TODO

    debugMsg("SimulatorScriptReader:parseCommandDeclaration",
             ' ' << word);

    // Parse parameter declarations
    // NYI

    // Construct and return the symbol
    return new SimSymbol(word, COMMAND_SYM_TYPE, returnType);
  }

  Value parseReturnValue(ValueType returnType)
  {
    std::string const &line = m_instream.getLineStream().str();

    switch (returnType) {
    case BOOLEAN_TYPE: {
      Boolean bv;
      parseValue(line, bv);
      return Value(bv);
    }

    case INTEGER_TYPE: {
      Integer iv;
      parseValue(line, iv);
      return Value(iv);
    }

    case REAL_TYPE: {
      Real rv;
      parseValue(line, rv);
      return Value(rv);
    }

    case STRING_TYPE: {
      String sv;
      parseValue(line, sv);
      return Value(sv);
    }

    case COMMAND_HANDLE_TYPE:
      return Value((uint16_t) parseCommandHandleValue(line));

    case DATE_TYPE:
    case DURATION_TYPE:
    case ARRAY_TYPE:
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:

      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": unimplemented return value type "
                << valueTypeName(returnType)
                << std::endl;
      return Value();

    default:
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": invalid return value type " << returnType
                << std::endl;
      return Value();
    }
  }

  bool parseTelemetryReturn(std::string name,
                            SimSymbol const *symbol,
                            bool compatibilityMode = false)
  {
    debugMsg("SimulatorScriptReader:parseTelemetryReturn",
             ' ' << name << (compatibilityMode ? " (compatibility mode)" : ""));
    
    // Construct the ResponseMessage and add it to the agenda
    double delay;
    std::istringstream &linestream = m_instream.getLineStream();
    linestream >> delay;
    if (linestream.fail()) {
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": parse error in telemetry delay for "
                << name
                << std::endl;
      return false;
    }
    Value returnValue;
    // Return value is on next line
    m_instream.getLine();
    if (compatibilityMode) {
      Real realVal;
      linestream >> realVal;
      if (linestream.fail()) {
        std::cerr << "Error: file " << m_instream.getFileName()
                  << ", line " << m_instream.getLineCount()
                  << ": parse error in telemetry return for "
                  << name
                  << std::endl;
        return false;
      }
      returnValue = realVal;
    }
    else {
      returnValue = parseReturnValue(symbol->returnType);
      // Error handling??
    }

    timeval timeDelay = doubleToTimeval(delay);
    debugMsg("SimulatorScriptReader:readScript",
             " Adding telemetry for " << name
             << " at delay " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);

    m_agenda->scheduleResponse(timeDelay,
                               new ResponseMessage(name, returnValue, MSG_TELEMETRY));
    return true;
  }

  bool parseCommandReturn(std::string name,
                          SimSymbol const *symbol,
                          bool compatibilityMode = false)
  {
    debugMsg("SimulatorScriptReader:parseCommandReturn",
             ' ' << name << (compatibilityMode ? " (compatibility mode)" : ""));
    
    // Construct the GenericResponse and add it to the manager map
    unsigned long commandIndex;
    unsigned int numOfResponses;
    double delay;
    std::istringstream &linestream = m_instream.getLineStream();
    linestream >> commandIndex;
    if (linestream.fail()) {
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": parse error in command index for "
                << name
                << std::endl;
      return false;
    }
    linestream >> numOfResponses;
    if (linestream.fail()) {
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": parse error in command number of responses for "
                << name
                << std::endl;
      return false;
    }
    linestream >> delay;
    if (linestream.fail()) {
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": parse error in command response delay for "
                << name
                << std::endl;
      return false;
    }

    Value returnValue;
    // Return value is on next line
    m_instream.getLine();
    if (m_instream.eof()) {
      std::cerr << "Error: file " << m_instream.getFileName()
                << ", line " << m_instream.getLineCount()
                << ": premature end of file reading return value for "
                << name
                << std::endl;
      return false;
    }

    // TODO: check IO errors
    if (compatibilityMode) {
      Real realVal;
      linestream >> realVal;
      if (linestream.fail()) {
        std::cerr << "Error: file " << m_instream.getFileName()
                  << ", line " << m_instream.getLineCount()
                  << ": parse error in command return for "
                  << name
                  << std::endl;
        return false;
      }
      returnValue = realVal;
    }
    else {
      returnValue = parseReturnValue(symbol->returnType);
      // TODO: error handling
    }      

    timeval timeDelay = doubleToTimeval(delay);
    debugMsg("SimulatorScriptReader:readScript",
             " Adding command return for " << name
             << " index " << commandIndex
             << " at interval " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);

    GenericResponse *resp = new GenericResponse(name, returnValue, timeDelay, numOfResponses);
    ensureResponseMessageManager(name)->addResponse(resp, commandIndex);
    return true;
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
      return it->second;
    }

    CommandResponseManager* result = new CommandResponseManager(name);
    (*m_map)[name] = result;
    return result;
  }

};

SimulatorScriptReader *makeScriptReader(ResponseManagerMap *map,
                                        Agenda *agenda)
{
  return new PlexilSimScriptReader(map, agenda);
}
