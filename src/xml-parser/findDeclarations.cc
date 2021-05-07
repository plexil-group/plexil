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

#include "findDeclarations.hh"

#include "Debug.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

using namespace pugi;

namespace PLEXIL
{
  
  // Search upward from elt for a Node.
  xml_node const findContainingNodeElement(xml_node const elt)
  {
    xml_node candidate = elt.parent();
    while (candidate.type() == node_element) {
      debugMsg("findContainingNodeElement", " at " << candidate.name());
      if (testTag(NODE_TAG, candidate)) {
        debugMsg("findContainingNodeElement",
                 " found " << candidate.name()
                 << ' ' << candidate.child_value(NODEID_TAG));
        return candidate;
      }
      candidate = candidate.parent();
    }
    debugMsg("findContainingNodeElement", " not found");
    return xml_node();
  }

  // look for a declaration with the given name in the element.
  xml_node const findNameInDeclarations(xml_node const elt,
                                        char const *name)
  {
    xml_node temp = elt.first_child();
    while (temp) {
      xml_node const nameElt = temp.child(NAME_TAG);
      if (nameElt && !strcmp(nameElt.child_value(), name))
        return temp;
      temp = temp.next_sibling();
    }
    return xml_node();
  }

  // look for a declaration with the given tag and name in the element.
  xml_node const findTagInDeclarations(xml_node const elt,
                                       char const *tag,
                                       char const *name)
  {
    xml_node temp = elt.child(tag);
    while (temp) {
      xml_node const nameElt = temp.child(NAME_TAG);
      // If we trip over this, we missed a check up the tree.
      checkParserExceptionWithLocation(nameElt,
                                       temp,
                                       "Internal error: " << tag
                                       << " with no " << NAME_TAG << " element");
      if (!strcmp(nameElt.child_value(), name))
        return temp;
      temp = temp.next_sibling(tag);
    }
    return xml_node();
  }
  
  // Find the first in-scope variable declaration with the given variable name.
  xml_node const findVariableDeclaration(xml_node const elt, char const *name)
  {
    debugMsg("findVariableDeclaration", " for \"" << name << '"');

    // Search upward from elt for a Node with a variable defined in a
    // VariableDeclarations, Interface/In, or Interface/InOut element
    xml_node node = findContainingNodeElement(elt);
    while (node) {
      debugMsg("findVariableDeclaration",
               " \"" << name << "\" checking node " << node.child_value(NODEID_TAG));

      xml_node temp = node.child(VAR_DECLS_TAG);
      if (temp) {
        xml_node const candidate = findTagInDeclarations(temp, DECL_VAR_TAG, name);
        if (candidate) {
          debugMsg("findVariableDeclaration",
                   " \"" << name << "\" found in node " << node.child_value(NODEID_TAG));
          return candidate;
        }
      }
      // Check for interface variable
      temp = node.child(INTERFACE_TAG);
      if (temp) {
        xml_node child = temp.first_child();
        while (child) {
          debugMsg("findVariableDeclaration",
                   " \"" << name << "\" checking " << child.name());
          xml_node const candidate = findTagInDeclarations(child, DECL_VAR_TAG, name);
          if (candidate) {
            debugMsg("findVariableDeclaration",
                     " \"" << name << "\" found in " << child.name() <<
                     " interface of node " << node.child_value(NODEID_TAG));
            return candidate;
          }
          child = child.next_sibling();
        }
      }
      node = findContainingNodeElement(node);
    }
    debugMsg("findVariableDeclaration", " \"" << name << "\" not found");
    return xml_node();
  }

  // Find the first in-scope array declaration with the given variable name.
  xml_node const findArrayDeclaration(xml_node const elt, char const *name)
  {
    debugMsg("findArrayDeclaration", " for \"" << name << '"');

    // Search upward from elt for a Node with an array variable defined in a
    // VariableDeclarations, Interface/In, or Interface/InOut element
    xml_node node = findContainingNodeElement(elt);
    while (node) {
      debugMsg("findArrayDeclaration",
               " \"" << name << "\" checking node " << node.child_value(NODEID_TAG));

      xml_node temp = node.child(VAR_DECLS_TAG);
      if (temp) {
        xml_node const candidate = findTagInDeclarations(temp, DECL_ARRAY_TAG, name);
        if (candidate) {
          debugMsg("findArrayDeclaration",
                   " \"" << name << "\" found in node " << node.child_value(NODEID_TAG));
          return candidate;
        }
      }
      // Check for interface variable
      temp = node.child(INTERFACE_TAG);
      if (temp) {
        xml_node child = temp.first_child();
        while (child) {
          debugMsg("findArrayDeclaration",
                   " \"" << name << "\" checking " << child.name());
          xml_node const candidate = findTagInDeclarations(child, DECL_ARRAY_TAG, name);
          if (candidate) {
            debugMsg("findArrayDeclaration",
                     " \"" << name << "\" found in " << child.name() <<
                     " interface of node " << node.child_value(NODEID_TAG));
            return candidate;
          }
          child = child.next_sibling();
        }
      }
      node = findContainingNodeElement(node);
    }
    debugMsg("findArrayDeclaration", " \"" << name << "\" not found");
    return xml_node();
  }

} // namespace PLEXIL
