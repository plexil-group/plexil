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

#ifndef PLEXIL_PARSE_NODE_HH
#define PLEXIL_PARSE_NODE_HH

namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{
  class NodeImpl;

  /**
   * @brief Check the node's XML before taking any action
   * @param xml The DOM representation of the node's XML.
   * @note Throws ParserException in the event of a parse error.
   */
  extern void checkNode(pugi::xml_node const xml);

  /**
   * @brief Construct the node and all its children from the given XML DOM.
   * @param xml The DOM representation of the node's XML.
   * @param parent The node which is the parent of the returned value.
   * @return The node represented by the XML, with all its children and variables populated.
   * @note Presumes that checkNode() has already been called.
   */
  extern NodeImpl *constructNode(pugi::xml_node const xml, NodeImpl *parent);

  /**
   * @brief Construct all the expressions for the node and its children from the given XML DOM.
   * @param node The node to finalize.
   * @param xml The DOM representation of the node's XML.
   */
  extern void finalizeNode(NodeImpl *node, pugi::xml_node const xml);

} // namespace PLEXIL

#endif // PLEXIL_PARSE_NODE_HH
