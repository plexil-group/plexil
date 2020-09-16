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

#ifndef PLEXIL_CREATE_EXPRESSION_HH
#define PLEXIL_CREATE_EXPRESSION_HH

#include "ValueType.hh"

namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{
  class Assignable;
  class Expression;
  class NodeConnector;

  /**
   * @brief Check the XML for validity as an expression.
   * @param nodeId Name of the node to which the expression belongs.
   * @param expr The XML representation of the expression.
   * @return The ValueType of the expression; UNKNOWN_TYPE if could not be determined.
   * @note Throws ParserException if any problems are found.
   */

  extern ValueType checkExpression(char const *nodeId, pugi::xml_node const expr);

  /**
   * @brief Check the XML for validity as an assignable expression.
   * @param nodeId Name of the node to which the expression belongs.
   * @param expr The XML representation of the expression.
   * @return The ValueType of the expression; UNKNOWN_TYPE if could not be determined.
   * @note Throws ParserException if any problems are found.
   */

  extern ValueType checkAssignable(char const *nodeId, pugi::xml_node const expr);
  
  /**
   * @brief Creates a new Expression instance with the type associated with the
   *        given expression specification.
   * @param expr The expression specification.
   * @param node Node for name lookup.
   * @return Pointer to the new Expression. May not be unique.
   * @note Convenience wrapper.
   */

  extern Expression *createExpression(pugi::xml_node const expr,
                                      NodeConnector *node = NULL);

  /**
   * @brief Creates a new Expression instance with the type associated with the
   *        given expression prototype.
   * @param expr The expression spec.
   * @param node Node for name lookup.
   * @return Pointer to the new Expression. May not be unique.
   * @param wasCreated Reference to a boolean variable;
   *                   variable will be set to true if new object created, false otherwise.
   */
  // Used in AssignmentNode, CommandNode, LibraryCallNode, Node::createConditions
  extern Expression *createExpression(pugi::xml_node const expr,
                                      NodeConnector *node,
                                      bool& wasCreated,
                                      ValueType returnType = UNKNOWN_TYPE);

  // Used in AssignmentNode, CommandNode
  extern Assignable *createAssignable(pugi::xml_node const expr,
                                      NodeConnector *node,
                                      bool& wasCreated);

  /**
   * @brief Deallocate all factories
   */
  // FIXME
  // extern void purgeExpressionFactories();

} // namespace PLEXIL

#endif // PLEXIL_CREATE_EXPRESSION_HH
