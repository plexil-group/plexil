/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#include "CommandNode.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "Command.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"
#include "ExternalInterface.hh"
#include "Variables.hh"

namespace PLEXIL
{
  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  CommandNode::CommandNode(const PlexilNodeId& nodeProto,
                           const ExecConnectorId& exec, 
                           const NodeId& parent)
    : Node(nodeProto, exec, parent)
  {
    checkError(nodeProto->nodeType() == NodeType_Command,
               "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
               << "\" for a CommandNode");
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  CommandNode::CommandNode(const LabelStr& type,
                           const LabelStr& name, 
                           const NodeState state,
                           const ExecConnectorId& exec,
                           const NodeId& parent)
    : Node(type, name, state, exec, parent)
  {
    checkError(type == COMMAND(),
               "Invalid node type \"" << type.toString() << "\" for a CommandNode");

    // Create dummy command for unit test
    createDummyCommand();

    switch (m_state) {
    case EXECUTING_STATE:
      deactivatePostCondition();
      m_command->activate();
      break;

    case FINISHING_STATE:
      activateActionCompleteCondition();
      activateAncestorExitInvariantConditions();
      activateExitCondition();
      activateInvariantCondition();
      activatePostCondition();
      m_command->activate();
      break;

    case FAILING_STATE:
      activateAbortCompleteCondition();
      m_command->activate();
      break;

    default:
      break;
    }

  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  CommandNode::~CommandNode()
  {
    debugMsg("CommandNode:~CommandNode", " destructor for " << m_nodeId.toString());

    // MUST be called first, here. Yes, it's redundant with base class.
    cleanUpConditions();

    cleanUpNodeBody();
  }

  // Not useful if called from base class destructor!
  // Can be called redundantly, e.g. from ListNode::cleanUpChildConditions().
  void CommandNode::cleanUpNodeBody()
  {
    debugMsg("CommandNode:cleanUpNodeBody", " for " << m_nodeId.toString());

    if (m_command.isId()) {
      debugMsg("CommandNode:cleanUpNodeBody", "<" << m_nodeId.toString() << "> Removing command.");
      delete (Command*) m_command;
      m_command = CommandId::noId();
    }
  }

  // Specific behaviors for derived classes
  void CommandNode::specializedPostInit(const PlexilNodeId& node)
  {
    debugMsg("Node:postInit", "Creating command for node '" << m_nodeId.toString() << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilCommandBody>::convertable(node->body()),
               "Node is a command node but doesn't have a command body.");
    createCommand((PlexilCommandBody*)node->body());
    m_variablesByName[COMMAND_HANDLE()] = m_command->getAck();

    // Construct action-complete condition
    ExpressionId actionComplete = (new IsKnown(m_command->getAck()))->getId();
    actionComplete->addListener(m_listener.getId());
    m_conditions[actionCompleteIdx] = actionComplete;
    m_garbageConditions[actionCompleteIdx] = true;

    // Construct command-aborted condition
    ExpressionId commandAbort = (ExpressionId) m_command->getAbortComplete();
    commandAbort->addListener(m_listener.getId());
    m_conditions[abortCompleteIdx] = commandAbort;
    m_garbageConditions[abortCompleteIdx] = false;
  }

  void CommandNode::createConditionWrappers()
  {
    // No need to wrap if end condition is default - (True || anything) == True
    if (m_conditions[endIdx] != BooleanVariable::TRUE_EXP()) {
      // Construct real end condition by wrapping existing
      removeConditionListener(endIdx);
      ExpressionId realEndCondition =
        (new Disjunction((new InterruptibleCommandHandleValues(m_command->getAck()))->getId(),
                         true,
                         m_conditions[endIdx],
                         m_garbageConditions[endIdx]))->getId();
      realEndCondition->addListener(m_listener.getId());
      m_conditions[endIdx] = realEndCondition;
      m_garbageConditions[endIdx] = true;
    }
  }

  //
  // State transition logic
  //

  //
  // EXECUTING 
  // 
  // Legal predecessor states: WAITING
  // Conditions active: AncestorExit, AncestorInvariant, End, Exit, Invariant
  // Legal successor states: FAILING, FINISHING

  void CommandNode::transitionToExecuting()
  {
    activateInvariantCondition();
    activateEndCondition();
  }

  NodeState CommandNode::getDestStateFromExecuting()
  {
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Ancestor exit for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FAILING. Command node and ancestor exit true.");
        return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Exit for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FAILING. Command node and exit true.");
        return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::FALSE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FAILING. Command node and ancestor invariant false.");
        return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Invariant for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::FALSE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FAILING. Command node and invariant false.");
        return FAILING_STATE;
    }

    cond = getEndCondition();
    checkError(cond->isActive(),
               "End for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FINISHING.  Command node and end condition true.");
      return FINISHING_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << 
             "' destination from EXECUTING: no state."
             << "\n  Ancestor exit: " << getAncestorExitCondition()->toString() 
             << "\n  Exit: " << getExitCondition()->toString() 
             << "\n  Ancestor invariant: " << getAncestorInvariantCondition()->toString() 
             << "\n  Invariant: " << getInvariantCondition()->toString() 
             << "\n  End: " << getEndCondition()->toString());
    return NO_NODE_STATE;
  }

  void CommandNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FINISHING_STATE ||
               destState == FAILING_STATE,
               "Attempting to transition Command node from EXECUTING to invalid state '"
               << StateVariable::nodeStateName(destState) << "'");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
    }
    else if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::EXITED());
    }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::PARENT_FAILED());
    }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
    }

    deactivateEndCondition();
    if (destState == FAILING_STATE) {
      deactivateExitCondition();
      deactivateInvariantCondition();
      deactivateAncestorExitInvariantConditions();
    }
  }

  //
  // FINISHING
  //
  // Legal predecessor states: EXECUTING
  // Conditions active: ActionComplete, AncestorExit, AncestorInvariant, Exit, Invariant, Post
  // Legal successor states: FAILING, ITERATION_ENDED

  void CommandNode::transitionToFinishing()
  {
    activateActionCompleteCondition();
    activatePostCondition();
  }

  NodeState CommandNode::getDestStateFromFinishing()
  {
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Ancestor exit for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Command node and ancestor exit true.");
      return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Exit for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Command node and exit true.");
      return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Command node and ancestor invariant false.");
      return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Invariant for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Command node, invariant false and end false or unknown.");
      return FAILING_STATE;
    }

    cond = getActionCompleteCondition();
    checkError(cond->isActive(),
               "Action complete for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: ITERATION_ENDED.  Command node and action complete true.");
      return ITERATION_ENDED_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << 
             "' destination from FINISHING: no state."
             << "\n  Ancestor exit: " << getAncestorExitCondition()->toString() 
             << "\n  Exit: " << getExitCondition()->toString()
             << "\n  Ancestor invariant: " << getAncestorInvariantCondition()->toString() 
             << "\n  Invariant: " << getInvariantCondition()->toString()
             << "\n  Action complete: " << getActionCompleteCondition()->toString());
    return NO_NODE_STATE;
  }

  void CommandNode::transitionFromFinishing(NodeState destState)
  {
    checkError(isPostConditionActive(),
               "Post for " << getNodeId().toString() << " is inactive.");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
    }
    else if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::EXITED());
    }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::PARENT_FAILED());
    }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
    }
    else if (getPostCondition()->getValue() != BooleanVariable::TRUE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::POST_CONDITION_FAILED());
    }
    else {
      m_outcomeVariable->setValue(OutcomeVariable::SUCCESS());
    }

    deactivateActionCompleteCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();
    if (destState == FAILING_STATE) {
      deactivateAncestorExitInvariantConditions();
    }
    else { // ITERATION_ENDED
      activateAncestorEndCondition();
      deactivateExecutable();
    }
  }


  //
  // FAILING
  //
  // Legal predecessor states: EXECUTING, FINISHING
  // Conditions active: AbortComplete
  // Legal successor states: FINISHED, ITERATION_ENDED

  void CommandNode::transitionToFailing()
  {
    activateAbortCompleteCondition();
    abort();
  }

  NodeState CommandNode::getDestStateFromFailing()
  {
    ExpressionId cond = getAbortCompleteCondition();
    checkError(cond->isActive(),
               "Abort complete for " << getNodeId().toString() << " is inactive.");
    if (cond->getValue() == BooleanVariable::TRUE_VALUE()) {
      if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_FAILED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FINISHED.  Command node abort complete, " <<
                 "and parent failed.");
        return FINISHED_STATE;
      }
      else if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_EXITED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FINISHED.  Command node abort complete, " <<
                 "and parent exited.");
        return FINISHED_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: ITERATION_ENDED.  Command node abort complete.");
        return ITERATION_ENDED_STATE;
      }
    }

    debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: no state.");
    return NO_NODE_STATE;
  }

  void CommandNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Command node from FAILING to invalid state '"
               << StateVariable::nodeStateName(destState) << "'");

    deactivateAbortCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
    }

    deactivateExecutable();
  }

  // TODO: figure out if this should be activated on entering EXECUTING state
  void CommandNode::specializedActivateInternalVariables()
  {
  }

  void CommandNode::specializedHandleExecution()
  {
    checkError(m_command.isValid(), "CommandNode::specializedHandleExecution: Command is invalid");
    m_command->activate();
    m_command->fixValues();
    m_command->fixResourceValues();
    m_exec->enqueueCommand(m_command);
  }

  void CommandNode::abort()
  {
    checkError(m_command.isValid(), "CommandNode::abort: Command is invalid");
    // Handle stupid unit test
    if (m_exec->getExternalInterface().isId()) {
      m_exec->getExternalInterface()->invokeAbort(m_command);
    }
  }

  void CommandNode::specializedDeactivateExecutable()
  {
    checkError(m_command.isValid(), "CommandNode::specializedDeactivateExecutable: Command is invalid");
    m_command->deactivate();
  }

  void CommandNode::specializedReset()
  {
    checkError(m_command.isValid(), "CommandNode::specializedReset: Command is invalid");
    m_command->reset();
  }

  void CommandNode::createCommand(const PlexilCommandBody* command) 
  {
    checkError(command->state()->nameExpr().isValid(),
               "Attempt to create command with invalid name expression");

    PlexilStateId state = command->state();
    std::vector<ExpressionId> garbage;
    bool wasCreated = false;
    ExpressionId nameExpr = 
      ExpressionFactory::createInstance(LabelStr(state->nameExpr()->name()), 
                                        state->nameExpr(), 
                                        NodeConnector::getId(),
                                        wasCreated);
    if (wasCreated)
      garbage.push_back(nameExpr);

    std::vector<ExpressionId> args;
    for (std::vector<PlexilExprId>::const_iterator it = state->args().begin();
         it != state->args().end(); 
         ++it) {
      ExpressionId argExpr =
        ExpressionFactory::createInstance(LabelStr((*it)->name()), *it, NodeConnector::getId(), wasCreated);
      check_error(argExpr.isValid());
      args.push_back(argExpr);
      if (wasCreated)
        garbage.push_back(argExpr);
    }
    
    VariableId destVar;
    LabelStr dest_name;
    if (!command->dest().empty()) {
      const PlexilExprId& destExpr = command->dest()[0]->getId();
      dest_name = destExpr->name();
      if (Id<PlexilVarRef>::convertable(destExpr)) {
        destVar = findVariable((Id<PlexilVarRef>) destExpr);
        // FIXME: push this check up into XML parser
        checkError(destVar.isValid(),
                   "Unknown destination variable '" << dest_name.toString() <<
                   "' in command in node '" <<
                   m_nodeId.toString() << "'");
      }
      else if (Id<PlexilArrayElement>::convertable(destExpr)) {
        destVar = ExpressionFactory::createInstance(LabelStr(destExpr->name()),
                                                    destExpr,
                                                    NodeConnector::getId());
        garbage.push_back(destVar);
      }
      else {
        checkError(ALWAYS_FAIL, "Invalid left-hand side for a command");
      }
    }

    // Resource
    ResourceList resourceList;
    const std::vector<PlexilResourceId>& plexilResourceList = command->getResource();
    for(std::vector<PlexilResourceId>::const_iterator resListItr = plexilResourceList.begin();
        resListItr != plexilResourceList.end(); ++resListItr) {
      ResourceMap resourceMap;

      const PlexilResourceMap& resources = (*resListItr)->getResourceMap();
      for (PlexilResourceMap::const_iterator resItr = resources.begin();
           resItr != resources.end();
           ++resItr) {
        bool wasCreated = false;
        ExpressionId resExpr
          = ExpressionFactory::createInstance(LabelStr(resItr->second->name()),
                                              resItr->second, 
                                              NodeConnector::getId(),
                                              wasCreated);
        check_error(resExpr.isValid());
        resourceMap[resItr->first] = resExpr;
        if (wasCreated)
          garbage.push_back(resExpr);
      }
      resourceList.push_back(resourceMap);
    }

    debugMsg("Node:createCommand",
             "Creating command"
             << (nameExpr->getValue().isUnknown() ? "" : " '" + nameExpr->getValue().getStringValue() + "'")
             << " for node '" << m_nodeId.toString() << "'");
    m_command = (new Command(nameExpr, args, destVar, dest_name, garbage, resourceList, getId()))->getId();
  }

  // Unit test variant of above
  void CommandNode::createDummyCommand() 
  {
    ExpressionId nameExpr = (new StringVariable(Value("dummy"), true))->getId();
    std::vector<ExpressionId> garbage;
    garbage.push_back(nameExpr);
    // Empty arglist
    std::vector<ExpressionId> args;
    
    // No destination variable
    VariableId destVar;
    LabelStr dest_name;

    // No resource
    ResourceList resourceList;
    m_command = (new Command(nameExpr, args, destVar, dest_name, garbage, resourceList, getId()))->getId();
  }

  void CommandNode::printCommandHandle(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
    stream << indentStr << " Command handle: " <<
      m_command->getAck()->toString() << '\n';
  }

}
