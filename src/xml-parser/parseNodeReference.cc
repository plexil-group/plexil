/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "parseNodeReference.hh"

#include "NodeImpl.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  static NodeImpl *parseNodeRef(pugi::xml_node nodeRef, NodeImpl *node)
  {
    // parse directional reference
    checkAttr(DIR_ATTR, nodeRef);
    const char* dirValue = nodeRef.attribute(DIR_ATTR).value();

    if (!strcmp(dirValue, SELF_VAL))
      return dynamic_cast<NodeImpl *>(node);

    NodeImpl *result = NULL;
    if (!strcmp(dirValue, PARENT_VAL)) {
      result = node->getParentNode();
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: Parent node reference in root node "
                                       << node->getNodeId());
      return result;
    }

    const char *name = nodeRef.child_value();
    checkParserExceptionWithLocation(*name,
                                     nodeRef,
                                     "createExpression: Empty node name");
    if (!strcmp(dirValue, CHILD_VAL)) {
      result = node->findChild(name);
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: No child node named " << name 
                                       << " in node " << node->getNodeId());
      return result;
    }
    if (!strcmp(dirValue, SIBLING_VAL)) {
      NodeImpl *parent = node->getParentNode();
      checkParserExceptionWithLocation(parent,
                                       nodeRef,
                                       "createExpression: Sibling node reference from root node "
                                       << node->getNodeId());
      result = parent->findChild(name);
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: No sibling node named " << name 
                                       << " for node " << node->getNodeId());
      return result;
    }
    else {
      reportParserExceptionWithLocation(nodeRef,
                                        "XML parsing error: Invalid value for " << DIR_ATTR << " attibute \""
                                        << dirValue << "\"");
      return NULL;
    }
  }

  static NodeImpl *findLocalNodeId(char const *name, NodeImpl *node)
  {
    // search for node ID
    if (node->getNodeId() == name)
      return dynamic_cast<NodeImpl *>(node);
    // Check children, if any
    NodeImpl *result = node->findChild(name);
    if (result)
      return result;
    return NULL;
  }

  static NodeImpl *parseNodeId(pugi::xml_node nodeRef, NodeImpl *node)
  {
    // search for node ID
    char const *name = nodeRef.child_value();
    checkParserExceptionWithLocation(*name,
                                     nodeRef,
                                     "Empty or invalid " << nodeRef.name() << " element");
    NodeImpl *result = findLocalNodeId(name, node);
    if (result)
      return result;

    NodeImpl *parent = node->getParentNode();
    while (parent) {
      result = findLocalNodeId(name, parent);
      if (result)
        return result;
      parent = parent->getParentNode();
    }
    reportParserExceptionWithLocation(nodeRef.first_child(),
                                      "createExpression: No node named "
                                      << name
                                      << " reachable from node " << node->getNodeId());
    return NULL;
  }

  NodeImpl *parseNodeReference(pugi::xml_node nodeRef, NodeImpl *node)
  {
    const char* tag = nodeRef.name();
    checkParserExceptionWithLocation(*tag,
                                     nodeRef.parent(),
                                     "createExpression: Node reference is not an element");
    if (0 == strcmp(tag, NODEREF_TAG))
      return parseNodeRef(nodeRef, node);
    else if (0 == strcmp(tag, NODEID_TAG))
      return parseNodeId(nodeRef, node);
    reportParserExceptionWithLocation(nodeRef,
                                      "createExpression: Invalid node reference");
    return NULL;
  }

}
