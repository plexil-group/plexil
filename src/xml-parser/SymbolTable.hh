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

#ifndef PLEXIL_SYMBOL_TABLE_HH
#define PLEXIL_SYMBOL_TABLE_HH

#include "ValueType.hh" // includes plexil-stdint.h, string

#include <map>
#include <vector>

namespace PLEXIL
{
  enum SymbolType : uint8_t {
    NO_SYMBOL_TYPE = 0,
    COMMAND_TYPE,
    LOOKUP_TYPE,
    LIBRARY_NODE_TYPE,
    MUTEX_TYPE,
    // FUNCTION_TYPE, // future
      
    SYMBOL_TYPE_MAX
  };

  class Symbol final
  {
  public:
    Symbol();
    Symbol(Symbol const &) = default;
    Symbol(Symbol &&) = default;

    Symbol(char const *name, SymbolType t);

    Symbol &operator=(Symbol const &) = default;
    Symbol &operator=(Symbol &&) = default;

    ~Symbol() = default;

    std::string const &name() const;

    SymbolType symbolType() const;

    void setReturnType(ValueType t);
    ValueType returnType() const;

    // A Symbol may have 0 or more required, typed parameters.
    void addParameterType(ValueType t);
    ValueType parameterType(size_t n) const;
    size_t parameterCount() const;

    // A Symbol may also have an unlimited number of parameters of any type
    // following the required parameters.
    void setAnyParameters();
    bool anyParameters() const;

  private:

    std::string m_name;
    std::vector<ValueType> m_paramTypes;
    SymbolType m_symbolType;
    ValueType m_returnType;
    bool m_anyParams;
  };

  class LibraryNodeSymbol final
  {
  public:
    LibraryNodeSymbol() = default;
    LibraryNodeSymbol(LibraryNodeSymbol const &) = default;
    LibraryNodeSymbol(LibraryNodeSymbol &&) = default;

    LibraryNodeSymbol(char const *name);

    LibraryNodeSymbol &operator=(LibraryNodeSymbol const &) = default;
    LibraryNodeSymbol &operator=(LibraryNodeSymbol &&) = default;

    ~LibraryNodeSymbol() = default;

    std::string const &name() const;

    SymbolType symbolType() const;

    void addParameter(char const *pname, ValueType t, bool isInOut);

    bool isParameterDeclared(char const *pname);
    bool isParameterInOut(char const *pname);
    ValueType parameterValueType(char const *pname);

  private:

    std::string m_name;
    std::map<std::string, bool> m_paramInOutMap;
    std::map<std::string, ValueType> m_paramTypeMap;
  };
  
  class SymbolTable
  {
  public:
    virtual ~SymbolTable() = default;

    // These return nullptr if name is a duplicate.
    virtual Symbol *addCommand(char const *name) = 0;
    virtual Symbol *addLookup(char const *name) = 0;
    virtual Symbol *addMutex(char const *name) = 0;
    virtual LibraryNodeSymbol *addLibraryNode(char const *name) = 0;

    virtual Symbol const *getCommand(char const *name) = 0;
    virtual Symbol const *getLookup(char const *name) = 0;
    virtual Symbol const *getMutex(char const *name) = 0;
    virtual LibraryNodeSymbol const *getLibraryNode(char const *name) = 0;
  };

  extern SymbolTable *makeSymbolTable();

  // Set the current symbol table, saving the old value to restore later.
  extern void pushSymbolTable(SymbolTable *s);

  // Restore the previous symbol table.
  extern void popSymbolTable();

  //
  // Parser queries
  //

  extern Symbol const *getLookupSymbol(char const *name);
  extern Symbol const *getCommandSymbol(char const *name);
  extern LibraryNodeSymbol const *getLibraryNodeSymbol(char const *name);

}

#endif // PLEXIL_SYMBOL_TABLE_HH
