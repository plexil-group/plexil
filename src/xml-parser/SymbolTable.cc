/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "SymbolTable.hh"
#include "Mutex.hh"

#include "Debug.hh"

#include <memory>
#include <stack>

namespace PLEXIL
{
  //
  // Symbol
  //

  Symbol::Symbol()
    : m_name(),
      m_paramTypes(),
      m_symbolType(NO_SYMBOL_TYPE),
      m_returnType(UNKNOWN_TYPE),
      m_anyParams(false)
  {
  }

  Symbol::Symbol(char const *name, SymbolType t)
    : m_name(name),
      m_paramTypes(),
      m_symbolType(t),
      m_returnType(UNKNOWN_TYPE),
      m_anyParams(false)
  {
  }

  std::string const &Symbol::name() const
  {
    return m_name;
  }

  SymbolType Symbol::symbolType() const
  {
    return m_symbolType;
  }

  ValueType Symbol::returnType() const
  {
    return m_returnType;
  }

  ValueType Symbol::parameterType(size_t n) const
  {
    if (n > m_paramTypes.size())
      return UNKNOWN_TYPE;
    return m_paramTypes[n];
  }

  void Symbol::setReturnType(ValueType t)
  {
    m_returnType = t;
  }

  void Symbol::setAnyParameters()
  {
    m_anyParams = true;
  }

  bool Symbol::anyParameters() const
  {
    return m_anyParams;
  }

  void Symbol::addParameterType(ValueType t)
  {
    m_paramTypes.push_back(t);
  }

  size_t Symbol::parameterCount() const
  {
    return m_paramTypes.size();
  }

  //
  // LibraryNodeSymbol
  //

  LibraryNodeSymbol::LibraryNodeSymbol(char const *name)
    : m_name(name),
      m_paramInOutMap(),
      m_paramTypeMap()
  {
  }

  std::string const &LibraryNodeSymbol::name() const
  {
    return m_name;
  }

  SymbolType LibraryNodeSymbol::symbolType() const
  {
    return LIBRARY_NODE_TYPE;
  }

  void LibraryNodeSymbol::addParameter(char const *pname,
                                       ValueType t,
                                       bool isInOut)
  {
    std::string const pnameStr(pname);
    std::map<std::string, bool>::const_iterator it =
      m_paramInOutMap.find(pnameStr);
    if (it != m_paramInOutMap.end()) {
      // TODO Parser exception - duplicate name
    }
    m_paramInOutMap[pnameStr] = isInOut;
    m_paramTypeMap[pnameStr] = t;
  }

  bool LibraryNodeSymbol::isParameterDeclared(char const *pname)
  {
    std::string const pnameStr(pname);
    std::map<std::string, bool>::const_iterator it =
      m_paramInOutMap.find(pnameStr);
    return it != m_paramInOutMap.end();
  }

  bool LibraryNodeSymbol::isParameterInOut(char const *pname)
  {
    std::string const pnameStr(pname);
    std::map<std::string, bool>::const_iterator it =
      m_paramInOutMap.find(pnameStr);
    if (it == m_paramInOutMap.end())
      return false;
    return it->second;
  }

  ValueType LibraryNodeSymbol::parameterValueType(char const *pname)
  {
    std::string const pnameStr(pname);
    std::map<std::string, ValueType>::const_iterator it =
      m_paramTypeMap.find(pnameStr);
    if (it == m_paramTypeMap.end())
      return UNKNOWN_TYPE;
    return it->second;
  }

  //
  // SymbolTableImpl
  //

  class SymbolTableImpl
    : public SymbolTable
  {
  private:
    using SymbolMap = std::map<std::string, std::unique_ptr<Symbol>>;
    using LibraryMap = std::map<std::string, std::unique_ptr<LibraryNodeSymbol>>;

    SymbolMap m_commandMap;
    SymbolMap m_lookupMap;
    SymbolMap m_mutexMap;
    // SymbolMap m_functionMap; // future
    LibraryMap m_libraryMap;

  public:

    SymbolTableImpl() = default;

    ~SymbolTableImpl() = default;
    Symbol *addCommand(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_commandMap.find(namestr);
      if (it != m_commandMap.end())
        return nullptr; // duplicate
      Symbol *result = new Symbol(name, COMMAND_TYPE);
      m_commandMap.emplace(namestr, result);
      return result;
    }

    Symbol *addLookup(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_lookupMap.find(namestr);
      if (it != m_lookupMap.end())
        return nullptr; // duplicate
      Symbol *result = new Symbol(name, LOOKUP_TYPE);
      m_lookupMap.emplace(namestr, result);
      return result;
    }

    Symbol *addMutex(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_mutexMap.find(namestr);
      if (it != m_mutexMap.end())
        return nullptr; // duplicate

      ensureGlobalMutex(name); // for effect

      Symbol *result = new Symbol(name, MUTEX_TYPE);
      m_mutexMap.emplace(namestr, result);
      return result;
    }

    LibraryNodeSymbol *addLibraryNode(char const *name)
    {
      std::string const namestr(name);
      LibraryMap::const_iterator it =
        m_libraryMap.find(namestr);
      if (it != m_libraryMap.end())
        return nullptr; // duplicate
      LibraryNodeSymbol *result = new LibraryNodeSymbol(name);
      m_libraryMap.emplace(namestr, result);
      return result;
    }

    Symbol const *getCommand(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_commandMap.find(namestr);
      if (it == m_commandMap.end())
        return nullptr;
      return it->second.get();
    }

    Symbol const *getLookup(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_lookupMap.find(namestr);
      if (it == m_lookupMap.end())
        return nullptr;
      return it->second.get();
    }

    Symbol const *getMutex(char const *name)
    {
      std::string const namestr(name);
      SymbolMap::const_iterator it =
        m_mutexMap.find(namestr);
      if (it == m_mutexMap.end())
        return nullptr;
      return it->second.get();
    }

    LibraryNodeSymbol const *getLibraryNode(char const *name)
    {
      std::string const namestr(name);
      LibraryMap::const_iterator it =
        m_libraryMap.find(namestr);
      if (it == m_libraryMap.end())
        return nullptr;
      return it->second.get();
    }

  };

  SymbolTable *makeSymbolTable()
  {
    return new SymbolTableImpl();
  }

  static std::stack<SymbolTable *> s_symtabStack;

  static SymbolTable *s_symbolTable = nullptr;

  void pushSymbolTable(SymbolTable *s)
  {
    debugMsg("pushSymbolTable", ' ' << s);
    if (s_symbolTable)
      s_symtabStack.push(s_symbolTable);
    s_symbolTable = s;
  }

  void popSymbolTable()
  {
    debugMsg("popSymbolTable", ' ' << s_symbolTable);
    if (s_symtabStack.empty()) {
      // Back at top level
      s_symbolTable = nullptr;
      return;
    }
    else {
      s_symbolTable = s_symtabStack.top();
      s_symtabStack.pop();
    }
  }

  extern Symbol const *getLookupSymbol(char const *name)
  {
    if (s_symbolTable)
      return s_symbolTable->getLookup(name);
    else
      return nullptr;
  }

  extern Symbol const *getCommandSymbol(char const *name)
  {
    if (s_symbolTable)
      return s_symbolTable->getCommand(name);
    else
      return nullptr;
  }

  extern LibraryNodeSymbol const *getLibraryNodeSymbol(char const *name)
  {
    if (s_symbolTable)
      return s_symbolTable->getLibraryNode(name);
    else
      return nullptr;
  }

}
