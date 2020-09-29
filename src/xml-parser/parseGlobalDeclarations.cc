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

#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

using pugi::xml_node;

namespace PLEXIL
{

  static void checkValueDeclaration(xml_node const elt)
  {
    // We only care about Type and MaxSize
    xml_node typeElt = elt.child(TYPE_TAG);
    checkParserExceptionWithLocation(typeElt,
                                     elt,
                                     elt.name() << " with no " << TYPE_TAG << " element");

    char const *typnam = typeElt.child_value();
    checkParserExceptionWithLocation(typnam && *typnam,
                                     typeElt,
                                     elt.name() << " with empty " << TYPE_TAG << " element");

    // Allow "Any" type
    ValueType typ = parseValueType(typnam);
    if (typ == UNKNOWN_TYPE) {
      checkParserExceptionWithLocation(!strcmp(typnam, ANY_VAL),
                                       typeElt,
                                       elt.name() << " has invalid type name " << typnam);
    }

    // Is it an array?
    xml_node maxElt = typeElt.next_sibling();
    if (maxElt) {
      checkParserExceptionWithLocation(testTag(MAX_SIZE_TAG, maxElt),
                                       maxElt,
                                       "Invalid element " << maxElt.name() << " in " << elt.name());
      typ = arrayType(typ);
      checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                       typeElt,
                                       elt.name() << " has illegal array element type " << typnam);
    }
  }

  static void checkCommandDeclaration(xml_node const declXml)
  {
    xml_node elt = declXml.first_child();
    checkTag(NAME_TAG, elt);

    char const *name = elt.child_value();
    checkParserExceptionWithLocation(name && *name,
                                     elt,
                                     declXml.name() << " " << NAME_TAG << " is empty");

    elt = elt.next_sibling();
    if (!elt)
      return; // all done

    if (testTag(RETURN_TAG, elt)) {
      checkValueDeclaration(elt);
      elt = elt.next_sibling();
    }

    while (elt && testTag(PARAMETER_TAG, elt)) {
      checkValueDeclaration(elt);
      elt = elt.next_sibling();
    }

    if (elt && testTag(ANY_PARAMETERS_TAG, elt)) {
      elt = elt.next_sibling();
    }

    if (elt && testTag(RESOURCE_LIST_TAG, elt)) {
      // TODO check ResourceList
    }
  }

  static void checkStateDeclaration(xml_node const declXml)
  {
    xml_node elt = declXml.first_child();
    checkTag(NAME_TAG, elt);

    char const *name = elt.child_value();
    checkParserExceptionWithLocation(name && *name,
                                     elt,
                                     declXml.name() << " " << NAME_TAG << " is empty");

    elt = elt.next_sibling();
    checkTag(RETURN_TAG, elt);
    checkValueDeclaration(elt);
    elt = elt.next_sibling();

    while (elt && testTag(PARAMETER_TAG, elt)) {
      checkValueDeclaration(elt);
      elt = elt.next_sibling();
    }

    if (elt && testTag(ANY_PARAMETERS_TAG, elt)) {
      elt = elt.next_sibling();
    }

    // Barf if there are leftovers
    checkParserExceptionWithLocation(!elt,
                                     elt,
                                     "Extraneous content at end of " << STATE_DECLARATION_TAG);
  }

  static void checkInterfaceVariable(xml_node const varDecl, bool isInOut)
  {
    xml_node elt = varDecl.first_child();
    checkTag(NAME_TAG, elt);

    char const *varName = elt.child_value();
    checkParserExceptionWithLocation(varName && *varName,
                                     elt,
                                     "Empty " << NAME_TAG << " element in In"
                                     << (isInOut ? "Out" : "")
                                     << " declaration " << varDecl.name());

    elt = elt.next_sibling();
    checkTag(TYPE_TAG, elt);

    char const *typeName = elt.child_value();
    checkParserExceptionWithLocation(typeName && *typeName,
                                     varDecl,
                                     "Empty " << TYPE_TAG
                                     << " element in In"
                                     << (isInOut ? "Out" : "")
                                     << " interface variable " << varName);
    ValueType t = parseValueType(typeName);
    checkParserExceptionWithLocation(isScalarType(t),
                                     elt,
                                     "Invalid or illegal type " << typeName
                                     << " for In"
                                     << (isInOut ? "Out" : "")
                                     << " interface variable " << varName);
  }

  static void checkInterfaceArrayVariable(xml_node const varDecl, bool isInOut)
  {
    xml_node elt = varDecl.first_child();
    checkTag(NAME_TAG, elt);

    char const *varName = elt.child_value();
    checkParserExceptionWithLocation(varName && *varName,
                                     elt,
                                     "Empty " << NAME_TAG << " element in In"
                                     << (isInOut ? "Out" : "")
                                     << " declaration " << varDecl.name());

    elt = elt.next_sibling();
    checkTag(TYPE_TAG, elt);

    char const *typeName = elt.child_value();
    checkParserExceptionWithLocation(typeName && *typeName,
                                     elt,
                                     "Empty " << TYPE_TAG
                                     << " element for In"
                                     << (isInOut ? "Out" : "")
                                     << " interface array variable " << varName);
    ValueType t = parseValueType(typeName);
    ValueType at = arrayType(t);
    checkParserExceptionWithLocation(isScalarType(t) && at != UNKNOWN_TYPE,
                                     varDecl,
                                     "Invalid or illegal element type " << typeName
                                     << " for In"
                                     << (isInOut ? "Out" : "")
                                     << "interface array variable " << varName);
  }

  static void checkInDeclaration(xml_node const inDecl, char const *name)
  {
    for (xml_node decl = inDecl.first_child(); decl; decl = decl.next_sibling()) {
      if (testTag(DECL_VAR_TAG, decl))
        checkInterfaceVariable(decl, false);
      else if (testTag(DECL_ARRAY_TAG, decl))
        checkInterfaceArrayVariable(decl, false);
      else {
        reportParserExceptionWithLocation(decl,
                                          LIBRARY_NODE_DECLARATION_TAG << ' ' << name
                                          << ": Junk in " << inDecl.name() << " element");
      }
    }
  }

  static void checkInOutDeclaration(xml_node const inOutDecl, char const *name)
  {
    for (xml_node decl = inOutDecl.first_child(); decl; decl = decl.next_sibling()) {
      if (testTag(DECL_VAR_TAG, decl))
        checkInterfaceVariable(decl, true);
      else if (testTag(DECL_ARRAY_TAG, decl))
        checkInterfaceArrayVariable(decl, true);
      else {
        reportParserExceptionWithLocation(decl,
                                          LIBRARY_NODE_DECLARATION_TAG << ' ' << name
                                          << ": Junk in " << inOutDecl.name() << " element");
      }
    }
  }
  
  static void checkLibraryNodeDeclaration(xml_node const declXml)
  {
    xml_node elt = declXml.first_child();
    checkTag(NAME_TAG, elt);

    char const *name = elt.child_value();
    checkParserExceptionWithLocation(name && *name,
                                     elt,
                                     declXml.name() << " " << NAME_TAG << " is empty");

    // Interface
    elt = elt.next_sibling();
    if (testTag(INTERFACE_TAG, elt)) {
      for (xml_node child = elt.first_child();
           child;
           child = child.next_sibling()) {
        if (testTag(IN_TAG, child))
          checkInDeclaration(child, name);
        else if (testTag(INOUT_TAG, child))
          checkInOutDeclaration(child, name);
        else {
          reportParserExceptionWithLocation(child,
                                            declXml.name() << " " << name
                                            << ": illegal element " << child.name()
                                            << " in " << INTERFACE_TAG);
        }
      }
    }
  }

  void checkGlobalDeclarations(xml_node const &declsXml)
  {
    for (xml_node decl = declsXml.first_child();
         decl;
         decl = decl.next_sibling()) {
      if (testTag(COMMAND_DECLARATION_TAG, decl))
        checkCommandDeclaration(decl);
      else if (testTag(STATE_DECLARATION_TAG, decl))
        checkStateDeclaration(decl);
      else if (testTag(LIBRARY_NODE_DECLARATION_TAG, decl))
        checkLibraryNodeDeclaration(decl);
      else
        reportParserExceptionWithLocation(decl,
                                          "Unexpected element " << decl.name()
                                          << " found in " << declsXml.name());
    }
  }

  //
  // Second pass - actually build the global symbol table
  //

  static ValueType parseValueDeclaration(xml_node const elt)
  {
    // We only care about Type and MaxSize
    ValueType typ = parseValueType(elt.child_value(TYPE_TAG));
    // N.B. if typ is UNKNOWN_TYPE presume it's ANY

    // Is it an array?
    xml_node maxElt = elt.child(MAX_SIZE_TAG);
    if (maxElt)
      typ = arrayType(typ);
    return typ;
  }

  static void parseCommandDeclaration(xml_node const declXml, SymbolTable *symtab)
  {
    char const *name = declXml.child_value(NAME_TAG);
    Symbol *cmd = symtab->addCommand(name);
    checkParserExceptionWithLocation(cmd,
                                     declXml,
                                     declXml.name() << " " << name << " is already declared");

    xml_node elt = declXml.child(RETURN_TAG);
    if (elt) {
      cmd->setReturnType(parseValueDeclaration(elt));
    }

    elt = declXml.child(PARAMETER_TAG);
    while (elt) {
      cmd->addParameterType(parseValueDeclaration(elt));
      elt = elt.next_sibling(PARAMETER_TAG);
    }

    if (declXml.child(ANY_PARAMETERS_TAG)) {
      cmd->setAnyParameters();
      elt = elt.next_sibling();
    }

    elt = declXml.child(RESOURCE_LIST_TAG);
    if (elt) {
      // TODO parse ResourceList
    }
  }

  static void parseStateDeclaration(xml_node const declXml, SymbolTable *symtab)
  {
    char const *name = declXml.child_value(NAME_TAG);
    Symbol *state = symtab->addLookup(name);
    checkParserExceptionWithLocation(state,
                                     declXml,
                                     declXml.name() << " " << name << " is already declared");

    xml_node elt = declXml.child(RETURN_TAG);
    state->setReturnType(parseValueDeclaration(elt));

    elt = declXml.child(PARAMETER_TAG);
    while (elt) {
      state->addParameterType(parseValueDeclaration(elt));
      elt = elt.next_sibling(PARAMETER_TAG);
    }

    if (declXml.child(ANY_PARAMETERS_TAG)) {
      state->setAnyParameters();
    }
  }

  static void parseInterfaceVariable(xml_node const varDecl, bool isInOut, LibraryNodeSymbol *lib)
  {
    char const *varName = varDecl.child_value(NAME_TAG);
    checkParserExceptionWithLocation(!lib->isParameterDeclared(varName),
                                     varDecl,
                                     "Interface variable " << varName << " is already declared");

    char const *typeName = varDecl.child_value(TYPE_TAG);
    lib->addParameter(varName, parseValueType(typeName), isInOut);
  }

  static void parseInterfaceArrayVariable(xml_node const varDecl, bool isInOut, LibraryNodeSymbol *lib)
  {
    char const *varName = varDecl.child_value(NAME_TAG);
    checkParserExceptionWithLocation(!lib->isParameterDeclared(varName),
                                     varDecl,
                                     "Interface variable " << varName << " is already declared");

    char const *typeName = varDecl.child_value(TYPE_TAG);
    lib->addParameter(varName, arrayType(parseValueType(typeName)), isInOut);
  }

  static void parseInDeclaration(xml_node const inDecl, LibraryNodeSymbol *lib)
  {
    for (xml_node decl = inDecl.first_child(); decl; decl = decl.next_sibling()) {
      if (testTag(DECL_VAR_TAG, decl))
        parseInterfaceVariable(decl, false, lib);
      else if (testTag(DECL_ARRAY_TAG, decl))
        parseInterfaceArrayVariable(decl, false, lib);
    }
  }

  static void parseInOutDeclaration(xml_node const inOutDecl, LibraryNodeSymbol *lib)
  {
    for (xml_node decl = inOutDecl.first_child(); decl; decl = decl.next_sibling()) {
      if (testTag(DECL_VAR_TAG, decl))
        parseInterfaceVariable(decl, true, lib);
      else if (testTag(DECL_ARRAY_TAG, decl))
        parseInterfaceArrayVariable(decl, true, lib);
    }
  }
  
  static void parseLibraryNodeDeclaration(xml_node const declXml, SymbolTable *symtab)
  {
    char const *name = declXml.child_value(NAME_TAG);
    LibraryNodeSymbol *lib = symtab->addLibraryNode(name);
    checkParserExceptionWithLocation(lib,
                                     declXml,
                                     declXml.name() << " " << name << " is already declared");

    // Interface
    xml_node const elt = declXml.child(INTERFACE_TAG);
    if (elt) {
      for (xml_node child = elt.first_child();
           child;
           child = child.next_sibling()) {
        if (testTag(IN_TAG, child))
          parseInDeclaration(child, lib);
        else if (testTag(INOUT_TAG, child))
          parseInOutDeclaration(child, lib);
      }
    }
  }

  SymbolTable *parseGlobalDeclarations(xml_node const &declsXml)
  {
    SymbolTable *symtab = makeSymbolTable();
    try {
      for (xml_node decl = declsXml.first_child();
           decl;
           decl = decl.next_sibling()) {
        if (testTag(COMMAND_DECLARATION_TAG, decl))
          parseCommandDeclaration(decl, symtab);
        else if (testTag(STATE_DECLARATION_TAG, decl))
          parseStateDeclaration(decl, symtab);
        else if (testTag(LIBRARY_NODE_DECLARATION_TAG, decl))
          parseLibraryNodeDeclaration(decl, symtab);
      }

      return symtab;
    }
    catch (...) {
      delete symtab;
      throw;
    }
  }
}
