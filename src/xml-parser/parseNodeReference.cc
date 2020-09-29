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

#include "parseNodeReference.hh"

#include "NodeImpl.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL
{

  static pugi::xml_node getContainingNode(pugi::xml_node nodeRef)
  {
    // This didn't work, so do it the hard way
    // return nodeRef.select_node("ancestor::Node").node();
    pugi::xml_node temp = nodeRef;
    while ((temp = temp.parent()))
      if (!strcmp(NODE_TAG, temp.name()))
        return temp;

    // Failure return
    return pugi::xml_node();
  }

  // theNode must be a Plexil Node 
  static pugi::xml_node getNodeChild(pugi::xml_node theNode, char const *childName)
  {
    char const *ntype = theNode.attribute(NODETYPE_ATTR).value();
    if (!strcmp(ntype, NODELIST_TAG)) {
      pugi::xpath_node_set children = theNode.select_nodes("NodeBody/NodeList/Node");
      for (pugi::xpath_node_set::const_iterator iter = children.begin();
           iter != children.end();
           ++iter) {
        if (!strcmp(childName, iter->node().child_value(NODEID_TAG)))
          return iter->node();
      }
    }
    else if (!strcmp(ntype, LIBRARYNODECALL_TAG)) {
      pugi::xml_node temp = theNode.select_node("NodeBody/LibraryNodeCall").node();
      if (!strcmp(childName, temp.child_value(NODEID_TAG))) {
        return temp;
      }
    }

    // Fall-through return
    return pugi::xml_node();
  }

  static void checkNodeRef(pugi::xml_node nodeRef)
  {
    checkAttr(DIR_ATTR, nodeRef);
    char const *dirValue = nodeRef.attribute(DIR_ATTR).value();
    if (!strcmp(dirValue, SELF_VAL))
      return; // no need to check further

    // All other directions need the containing Node
    pugi::xml_node self = getContainingNode(nodeRef);
    if (!strcmp(dirValue, PARENT_VAL)) {
      pugi::xml_node parent = getContainingNode(self);
      checkParserExceptionWithLocation(parent,
                                       nodeRef,
                                       "Invalid node reference: root node has no " << PARENT_VAL);
      return;
    }

    const char *name = nodeRef.child_value();
    checkParserExceptionWithLocation(*name,
                                     nodeRef,
                                     "Invalid node reference: empty node name");
    if (!strcmp(dirValue, SIBLING_VAL)) {
      pugi::xml_node parent = getContainingNode(self);
      checkParserExceptionWithLocation(parent,
                                       nodeRef,
                                       "Invalid node reference: root node has no siblings");
      checkParserExceptionWithLocation(getNodeChild(parent, name),
                                       nodeRef,
                                       "Invalid node reference: node "
                                       << self.child_value(NODEID_TAG)
                                       << " has no sibling named " << name);
    }
    else if (!strcmp(dirValue, CHILD_VAL)) {
      checkParserExceptionWithLocation(getNodeChild(self, name),
                                       nodeRef,
                                       "Invalid node reference: node " 
                                       << self.child_value(NODEID_TAG)
                                       << " has no child named " << name);
    }
    else {
      reportParserExceptionWithLocation(nodeRef, "Invalid node reference");
    }
  }

  static void checkNodeId(pugi::xml_node nodeRef)
  {
    char const *name = nodeRef.child_value();
    checkParserExceptionWithLocation(*name,
                                     nodeRef,
                                     "Invalid node reference: empty or invalid "
                                     << nodeRef.name() << " element");

    pugi::xml_node theNode = getContainingNode(nodeRef);
    // Check against own name
    if (!strcmp(name, theNode.child_value(NODEID_TAG)))
      return;

    // Check children
    if (getNodeChild(theNode, name))
      return;

    // Check parent
    pugi::xml_node parent = getContainingNode(theNode);
    if (!strcmp(name, parent.child_value(NODEID_TAG)))
      return;

    // Check siblings
    if (getNodeChild(parent, name))
      return;

    // Check ancestors
    while ((parent = getContainingNode(parent))) {
      if (!strcmp(name, parent.child_value(NODEID_TAG)))
        return;
      if (getNodeChild(parent, name))
        return;
    }

    // Not found
    reportParserExceptionWithLocation(nodeRef,
                                      "Invalid node reference: No node named "
                                      << name
                                      << " reachable from node "
                                      << theNode.child_value(NODEID_TAG));
  }

  void checkNodeReference(pugi::xml_node nodeRef)
  {
    const char* tag = nodeRef.name();
    checkParserExceptionWithLocation(*tag,
                                     nodeRef.parent(),
                                     "createExpression: Node reference is not an element");
    if (0 == strcmp(tag, NODEREF_TAG))
      checkNodeRef(nodeRef);
    else if (0 == strcmp(tag, NODEID_TAG))
      checkNodeId(nodeRef);
    else {
      reportParserExceptionWithLocation(nodeRef,
                                        "createExpression: Invalid node reference");
    }
  }

  static NodeImpl *parseNodeRef(pugi::xml_node nodeRef, NodeImpl *node)
  {
    // parse directional reference
    const char* dirValue = nodeRef.attribute(DIR_ATTR).value();
    if (!strcmp(dirValue, SELF_VAL))
      return node;

    NodeImpl *result = NULL;
    if (!strcmp(dirValue, PARENT_VAL)) {
      result = node->getParentNode();
      assertTrueMsg(result,
                    "Internal error: Node " << node->getNodeId() << " has no parent");
      return result;
    }

    char const *name = nodeRef.child_value();
    assertTrueMsg(*name,
                  "Internal error: Empty node name in " << nodeRef.name());

    if (!strcmp(dirValue, CHILD_VAL)) {
      result = node->findChild(name);
      assertTrueMsg(result,
                    "Internal error: Node " << node->getNodeId()
                    << " has no child named " << name);
      return result;
    }
    if (!strcmp(dirValue, SIBLING_VAL)) {
      NodeImpl *parent = node->getParentNode();
      assertTrueMsg(parent,
                    "Internal error: Node " << node->getNodeId() << " has no parent");
      result = parent->findChild(name);
      assertTrueMsg(result,
                    "Internal error: Node " << node->getNodeId()
                    << " has no sibling named " << name);
      return result;
    }
    else {
      // Should have been caught by check()
      errorMsg("Internal error: Invalid value for "
               << DIR_ATTR << " attibute \"" << dirValue << "\"");
      return NULL;
    }
  }

  static NodeImpl *findLocalNodeId(char const *name, NodeImpl *node)
  {
    // search for node ID
    if (node->getNodeId() == name)
      return node;
    // Check children, if any
    return node->findChild(name);
  }

  static NodeImpl *parseNodeId(pugi::xml_node nodeRef, NodeImpl *node)
  {
    // search for node ID
    char const *name = nodeRef.child_value();
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
    // Should have been caught by checkNodeId()
    errorMsg("Internal error: No node named " << name
             << " reachable from node " << node->getNodeId());
    return NULL;
  }

  NodeImpl *parseNodeReference(pugi::xml_node nodeRef, NodeImpl *node)
  {
    const char* tag = nodeRef.name();
    // We trust that checkNodeReference() above has been called
    if (tag[4] == 'I')
      return parseNodeId(nodeRef, node);
    return parseNodeRef(nodeRef, node);
  }

}
