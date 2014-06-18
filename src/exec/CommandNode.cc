/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "BooleanOperators.hh"
#include "Command.hh"
#include "Comparisons.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"
#include "ExpressionConstants.hh"
#include "ExternalInterface.hh"
#include "Function.hh"

namespace PLEXIL
{
  /**
   * @class CommandHandleInterruptible
   * @brief An Operator that returns true if the command handle is interruptible, false if not.
   */

  class CommandHandleInterruptible : public OperatorImpl<bool>
  {
  public:
    CommandHandleInterruptible()
      : OperatorImpl<bool>("Interruptible")
    {
    }

    ~CommandHandleInterruptible()
    {
    }

    bool checkArgCount(size_t count) const
    {
      return count == 1;
    }

    bool operator()(bool &result, ExpressionId arg) const
    {
      uint16_t val;
      if (!arg->getValue(val)) // unknown
        return false;
      if (val == COMMAND_DENIED || val == COMMAND_FAILED)
        return true;
      return false;
    }

    DECLARE_OPERATOR_STATIC_INSTANCE(CommandHandleInterruptible, bool)

  private:
    // Not implemented
    CommandHandleInterruptible(const CommandHandleInterruptible &);
    CommandHandleInterruptible &operator=(const CommandHandleInterruptible &);
  };


  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  CommandNode::CommandNode(const PlexilNodeId& nodeProto,
                           const NodeId& parent)
    : Node(nodeProto, parent)
  {
    checkError(nodeProto->nodeType() == NodeType_Command,
               "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
               << "\" for a CommandNode");
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  CommandNode::CommandNode(const std::string& type,
                           const std::string& name, 
                           const NodeState state,
                           const NodeId& parent)
    : Node(type, name, state, parent)
  {
    checkError(type == COMMAND(),
               "Invalid node type \"" << type << "\" for a CommandNode");

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
    debugMsg("CommandNode:~CommandNode", " destructor for " << m_nodeId);

    // MUST be called first, here. Yes, it's redundant with base class.
    cleanUpConditions();

    cleanUpNodeBody();
  }

  // Not useful if called from base class destructor!
  // Can be called redundantly, e.g. from ListNode::cleanUpChildConditions().
  void CommandNode::cleanUpNodeBody()
  {
    debugMsg("CommandNode:cleanUpNodeBody", " for " << m_nodeId);

    if (m_command.isId()) {
      debugMsg("CommandNode:cleanUpNodeBody", "<" << m_nodeId << "> Removing command.");
      delete (Command*) m_command;
      m_command = CommandId::noId();
    }
  }

  // Specific behaviors for derived classes
  void CommandNode::specializedPostInit(const PlexilNodeId& node)
  {
    debugMsg("Node:postInit", "Creating command for node '" << m_nodeId << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilCommandBody>::convertable(node->body()),
               "Node is a command node but doesn't have a command body.");
    createCommand((PlexilCommandBody*)node->body());
    m_variablesByName[COMMAND_HANDLE()] = m_command->getAck();

    // Construct action-complete condition
    ExpressionId actionComplete =
      (new Function(IsKnown::instance(),
                    makeExprVec(std::vector<ExpressionId>(1, m_command->getAck()),
                                std::vector<bool>(1, false))))->getId();
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
    if (m_conditions[endIdx] != TRUE_EXP()) {
      // Construct real end condition by wrapping existing
      removeConditionListener(endIdx);
      ExpressionId realEndCondition =
        (new Function(BooleanOr::instance(),
                      (new Function(CommandHandleInterruptible::instance(),
                                    m_command->getAck(),
                                    false))->getId(),
                      m_conditions[endIdx],
                      true,
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
               "Ancestor exit for " << getNodeId() << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Command node and ancestor exit true.");
        return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Exit for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Command node and exit true.");
        return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Ancestor invariant for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Command node and ancestor invariant false.");
        return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Invariant for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Command node and invariant false.");
        return FAILING_STATE;
    }

    cond = getEndCondition();
    checkError(cond->isActive(),
               "End for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FINISHING.  Command node and end condition true.");
      return FINISHING_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId << 
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
               << nodeStateName(destState) << "'");

    bool temp;
    if ((getAncestorExitCondition()->getValue(temp)) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(PARENT_EXITED);
    }
    else if (getExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(EXITED);
    }
    else if (getAncestorInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(PARENT_FAILED);
    }
    else if (getInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(INVARIANT_CONDITION_FAILED);
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
               "Ancestor exit for " << getNodeId() << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Command node and ancestor exit true.");
      return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Exit for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Command node and exit true.");
      return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Ancestor invariant for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Command node and ancestor invariant false.");
      return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Invariant for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Command node, invariant false and end false or unknown.");
      return FAILING_STATE;
    }

    cond = getActionCompleteCondition();
    checkError(cond->isActive(),
               "Action complete for " << getNodeId() << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: ITERATION_ENDED.  Command node and action complete true.");
      return ITERATION_ENDED_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId << 
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
               "Post for " << getNodeId() << " is inactive.");

    bool temp;
    if (getAncestorExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(PARENT_EXITED);
    }
    else if (getExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(EXITED);
    }
    else if (getAncestorInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(PARENT_FAILED);
    }
    else if (getInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(INVARIANT_CONDITION_FAILED);
    }
    else if (!getPostCondition()->getValue(temp) || !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(POST_CONDITION_FAILED);
    }
    else {
      setNodeOutcome(SUCCESS_OUTCOME);
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
               "Abort complete for " << getNodeId() << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      if (getFailureType() == PARENT_FAILED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED.  Command node abort complete, " <<
                 "and parent failed.");
        return FINISHED_STATE;
      }
      else if (getFailureType() == PARENT_EXITED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED.  Command node abort complete, " <<
                 "and parent exited.");
        return FINISHED_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: ITERATION_ENDED.  Command node abort complete.");
        return ITERATION_ENDED_STATE;
      }
    }

    debugMsg("Node:getDestState",
                 " '" << m_nodeId << "' destination: no state.");
    return NO_NODE_STATE;
  }

  void CommandNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Command node from FAILING to invalid state '"
               << nodeStateName(destState) << "'");

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
    g_interface->enqueueCommand(m_command);
  }

  void CommandNode::abort()
  {
    checkError(m_command.isValid(), "CommandNode::abort: Command is invalid");
    // Handle stupid unit test
    if (g_interface.isId()) {
      g_interface->abortCommand(m_command);
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
    ExpressionId nameExpr = createExpression(state->nameExpr(), 
                                             NodeConnector::getId(),
                                             wasCreated);
    if (wasCreated)
      garbage.push_back(nameExpr);

    std::vector<ExpressionId> args;
    for (std::vector<PlexilExprId>::const_iterator it = state->args().begin();
         it != state->args().end(); 
         ++it) {
      ExpressionId argExpr =
        createExpression(*it, NodeConnector::getId(), wasCreated);
      check_error(argExpr.isValid());
      args.push_back(argExpr);
      if (wasCreated)
        garbage.push_back(argExpr);
    }
    
    Assignable *dest = NULL;
    if (!command->dest().empty()) {
      const PlexilExprId& destExpr = command->dest()[0]->getId();
      bool destCreated;
      dest = createAssignable(destExpr, NodeConnector::getId(), destCreated);
      if (destCreated)
        garbage.push_back(dest->getId()); // as ExpressionId
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
        ExpressionId resExpr = createExpression(resItr->second, 
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
             "Creating command for node '" << m_nodeId << "'");
    m_command = (new Command(nameExpr, args, garbage, dest, resourceList, getNodeId()))->getId();
  }

  // Unit test variant of above
  void CommandNode::createDummyCommand() 
  {
    // Empty arglist
    std::vector<ExpressionId> args;
    std::vector<ExpressionId> garbage;
    // No destination variable
    // No resource
    ResourceList resourceList;
    m_command = 
      (new Command(DUMMY_CMD_NAME().getId(), args, garbage, NULL, resourceList, getNodeId()))->getId();
  }

  void CommandNode::printCommandHandle(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
    stream << indentStr << " Command handle: " <<
      m_command->getAck()->toString() << '\n';
  }

}
