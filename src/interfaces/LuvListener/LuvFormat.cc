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

#include "LuvFormat.hh"

#include "Assignable.hh"
#include "Error.hh"
#include "NodeImpl.hh"
#include "NodeTransition.hh"

#include <iostream>

#include <cstddef> // size_t

namespace PLEXIL {

  //
  // Local constants
  //
  
  // XML formatting options
  static constexpr const unsigned int PUGI_FORMAT_OPTIONS = pugi::format_raw | pugi::format_no_declaration;

  // Literal strings
  static constexpr const char TRUE_STR[] = "true";
  static constexpr const char FALSE_STR[] = "false";

  // XML tags
  static constexpr const char PLAN_INFO_TAG[] = "PlanInfo";
  static constexpr const char PLEXIL_PLAN_TAG[] = "PlexilPlan";
  static constexpr const char PLEXIL_LIBRARY_TAG[] = "PlexilLibrary";
  static constexpr const char VIEWER_BLOCKS_TAG[] = "ViewerBlocks";

  static constexpr const char NODE_ID_TAG[] = "NodeId";
  static constexpr const char NODE_PATH_TAG[] = "NodePath";

  static constexpr const char NODE_STATE_UPDATE_TAG[] = "NodeStateUpdate";
  static constexpr const char NODE_STATE_TAG[] = "NodeState";
  static constexpr const char NODE_OUTCOME_TAG[] = "NodeOutcome";
  static constexpr const char NODE_FAILURE_TYPE_TAG[] = "NodeFailureType";
  static constexpr const char CONDITIONS_TAG[] = "Conditions";

  static constexpr const char ASSIGNMENT_TAG[] = "Assignment";
  static constexpr const char VARIABLE_TAG[] = "Variable";
  static constexpr const char VARIABLE_NAME_TAG[] = "VariableName";
  static constexpr const char VARIABLE_VALUE_TAG[] = "Value";

  //
  // Local utilities
  //

  static inline void simpleStartTag(std::ostream& s, const char* val) {
    s << '<' << val << ">";
  }

  static inline void endTag(std::ostream& s, const char* val) {
    s << "</" << val << ">";
  }

  /**
   * @brief Generate a simple XML element containing some text.
   * @param s The stream to write the element to.
   * @param tag The tag for the element.
   * @param text The text for the element.
   */
  void simpleTextElement(std::ostream& s,
                         const char* tag,
                         const char* text) {
    simpleStartTag(s, tag);
    s << text;
    endTag(s, tag);
  }

  //* Internal function for formatNodePath
  void formatNodePathInternal(std::ostream& s, 
                              Node const *node) {
    // Fill in parents recursively
    if (node->getParent())
      formatNodePathInternal(s, node->getParent());
    // Put ours at the end
    simpleTextElement(s, NODE_ID_TAG, node->getNodeId().c_str());
  }

  /**
   * @brief Generate the XML representation of the path to the node.
   * @param s The stream to write the XML to.
   * @param node The plan node whose path is being constructed.
   */
  void formatNodePath(std::ostream& s, 
                      Node const *node) {
    simpleStartTag(s, NODE_PATH_TAG);
    formatNodePathInternal(s, node);
    endTag(s, NODE_PATH_TAG);
  }

  /**
   * @brief Generate the XML representation of the current values of the node's conditions.
   * @param s The stream to write the XML to.
   * @param node The node whose conditions are being extracted.
   */
  void formatConditions(std::ostream& s, 
                        Node const *nptr)
  {
    NodeImpl const *node = dynamic_cast<NodeImpl const *>(nptr);
    assertTrueMsg(node,
                  "LuvFormat::formatConditions: not a node");

    simpleStartTag(s, CONDITIONS_TAG);

    for (size_t i = 0; i < NodeImpl::conditionIndexMax; ++i) {
      Expression const *cond = node->getCondition(i);
      if (cond) {
        std::string const valueStr = cond->valueString();
        simpleTextElement(s, 
                          NodeImpl::ALL_CONDITIONS[i],
                          valueStr.c_str());
      }
    }

    endTag(s, CONDITIONS_TAG);
  }

  /**
   * @brief Construct the PlanInfo header XML.
   * @param s The stream to write the XML to.
   * @param block Whether the viewer should block.
   */
  void LuvFormat::formatPlanInfo(std::ostream& s, 
                                 bool block) {
    simpleStartTag(s, PLAN_INFO_TAG);
    simpleTextElement(s, 
                      VIEWER_BLOCKS_TAG,
                      (block ? TRUE_STR : FALSE_STR));
    endTag(s, PLAN_INFO_TAG);
  }

  /**
   * @brief Construct the node state transition XML.
   * @param s The stream to write the XML to.
   * @param prevState The state from which the node is transitioning.
   * @param node The node.
   */
  void LuvFormat::formatTransition(std::ostream& s, 
                                   NodeTransition const &trans)
  {
    simpleStartTag(s, NODE_STATE_UPDATE_TAG);

    // add state
    simpleTextElement(s, NODE_STATE_TAG, nodeStateName(trans.newState).c_str());

    // add outcome
    if (trans.node->getOutcome() != NO_OUTCOME)
      simpleTextElement(s, NODE_OUTCOME_TAG,
                        outcomeName(trans.node->getOutcome()).c_str());

    // add failure type
    if (trans.node->getFailureType() != NO_FAILURE)
      simpleTextElement(s, NODE_FAILURE_TYPE_TAG,
                        failureTypeName(trans.node->getFailureType()).c_str());
      
    // add the condition states
    formatConditions(s, trans.node);

    // add the path
    formatNodePath(s, trans.node);

    endTag(s, NODE_STATE_UPDATE_TAG);
  }

  /**
   * @brief Construct the assignment XML.
   * @param s The stream to write the XML to.
   * @param dest The expression being assigned to.
   * @param destName The variable name of the expression.
   * @param value The internal representation of the new value.
   */
  void LuvFormat::formatAssignment(std::ostream &s, 
                                   Expression const * /* dest */,
                                   std::string const &destName,
                                   Value const &value) {
    simpleStartTag(s, ASSIGNMENT_TAG);

    // format variable name
    simpleStartTag(s, VARIABLE_TAG);

    // TODO: get path to owning node, if any

    // get variable name
    simpleTextElement(s, VARIABLE_NAME_TAG, destName.c_str());
    endTag(s, VARIABLE_TAG);

    // format variable value
    std::string const valueStr = value.valueToString();
    simpleTextElement(s, 
                      VARIABLE_VALUE_TAG,
                      valueStr.c_str());
    
    endTag(s, ASSIGNMENT_TAG);
  }

  /**
   * @brief Format the message representing a new plan.
   * @param s The stream to write the XML to.
   * @param plan The XML DOM representation of the new plan.
   */
  void LuvFormat::formatPlan(std::ostream& s, 
                             pugi::xml_node const plan) {
    plan.print(s, "", PUGI_FORMAT_OPTIONS);
  }

  /**
   * @brief Construct the message representing a new library node.
   * @param s The stream to write the XML to.
   * @param plan The intermediate representation of the library node.
   */
  void LuvFormat::formatLibrary(std::ostream& s, 
                                pugi::xml_node const libNode)
  {
    // create a PLEXIL Library wrapper and stick the library node in it
    simpleStartTag(s, PLEXIL_LIBRARY_TAG);
    libNode.print(s, "", PUGI_FORMAT_OPTIONS);
    endTag(s, PLEXIL_LIBRARY_TAG);
  }

}
