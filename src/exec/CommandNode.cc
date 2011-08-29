/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
	: Node(nodeProto, exec, parent),
	  m_ack((new StringVariable(StringVariable::UNKNOWN()))->getId()),
	  m_commandHandleVariable((new CommandHandleVariable())->getId())
  {
	checkError(nodeProto->nodeType() == NodeType_Command,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
			   << "\" for a CommandNode");

	// Make ack and command handle pretty
	((VariableImpl*) m_ack)->setName(m_nodeId.toString() + " ack");
	((VariableImpl*) m_commandHandleVariable)->setName(m_nodeId.toString());
	// Make command handle accessible
	m_variablesByName[COMMAND_HANDLE().getKey()] = m_commandHandleVariable;
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  CommandNode::CommandNode(const LabelStr& type, const LabelStr& name, const NodeState state,
						   const bool skip, const bool start, const bool pre,
						   const bool invariant, const bool post, const bool end, const bool repeat,
						   const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
						   const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
						   const bool parentFinished, const bool cmdHdlRcvdCondition,
						   const ExecConnectorId& exec)
	: Node(type, name, state, 
		   skip, start, pre, invariant, post, end, repeat,
		   ancestorInvariant, ancestorEnd, parentExecuting, childrenFinished,
		   commandAbort, parentWaiting, parentFinished, cmdHdlRcvdCondition,
		   exec),
	  m_ack((new StringVariable(StringVariable::UNKNOWN()))->getId()),
	  m_commandHandleVariable((new CommandHandleVariable())->getId())
  {
	checkError(type == COMMAND(),
			   "Invalid node type \"" << type.toString() << "\" for a CommandNode");

	// Make command handle pretty
	((VariableImpl*) m_ack)->setName(m_nodeId.toString() + " ack");
	((VariableImpl*) m_commandHandleVariable)->setName(m_nodeId.toString());
	// Make command handle accessible
	m_variablesByName[COMMAND_HANDLE().getKey()] = m_commandHandleVariable;

	// Create dummy command for unit test
	createDummyCommand();
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  CommandNode::~CommandNode()
  {
	// Redundant with base class destructor
	cleanUpConditions();
	cleanUpNodeBody();
	// cleanUpVars(); // base destructor should handle this

	// Now safe to delete ack and command handle
	delete (Variable*) m_ack;
	m_ack = VariableId::noId();
	delete (Variable*) m_commandHandleVariable;
	m_commandHandleVariable = VariableId::noId();
  }

  // Not useful if called from base class destructor!
  void CommandNode::cleanUpNodeBody()
  {
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
  }

  void CommandNode::createSpecializedConditions()
  {
	// Construct command-aborted condition
	VariableId commandAbort = (new BooleanVariable())->getId();
	ExpressionListenerId abortListener = m_listeners[abortCompleteIdx];
	commandAbort->addListener(abortListener);
	m_conditions[abortCompleteIdx] = commandAbort;
          
	// Listen to any change in the command handle so that the internal variable 
	// CommandHandleVariable can be updated
	ExpressionId commandHandleCondition = (new AllCommandHandleValues(m_ack))->getId();
	commandHandleCondition->ignoreCachedValue();
	commandHandleCondition->addListener(m_listeners[commandHandleReceivedIdx]);
	m_conditions[commandHandleReceivedIdx] = commandHandleCondition;
	m_garbageConditions[commandHandleReceivedIdx] = true;
  }

  void CommandNode::createConditionWrappers()
  {
	// Construct real end condition
	m_conditions[endIdx]->removeListener(m_listeners[endIdx]);
	ExpressionId interruptEndCond = (new InterruptibleCommandHandleValues(m_ack))->getId();
	ExpressionId conjunctCondition = (new Conjunction((new IsKnown(m_ack))->getId(),
													  true, 
													  m_conditions[endIdx],
													  m_garbageConditions[endIdx]))->getId();
	ExpressionId realEndCondition =
	  (new Disjunction(interruptEndCond, true, conjunctCondition, true))->getId();
	realEndCondition->addListener(m_listeners[endIdx]);
	m_conditions[endIdx] = realEndCondition;
	m_garbageConditions[endIdx] = true;
  }

  //
  // Next-state logic
  //

  NodeState CommandNode::getDestStateFromExecuting()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isEndConditionActive(),
			   "End for " << getNodeId().toString() << " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
		if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
            debugMsg("Node:getDestState",
                     "Destination: FINISHED.  Ancestor invariant condition false and end " <<
                     "condition true.. ");
            return FINISHED_STATE;
		  }
		else {
		  debugMsg("Node:getDestState",
				   "Destination: FAILING.  Ancestor invariant condition false and end " <<
				   "condition false or unknown.");
		  return FAILING_STATE;
		}
      }
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
		if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		  debugMsg("Node:getDestState",
				   "Destination: ITERATION_ENDED.  Invariant condition false and end " <<
				   "condition true.. ");
		  return ITERATION_ENDED_STATE;
		}
		else {
            debugMsg("Node:getDestState",
                     "Destination: FAILING.  Invariant condition false and end condition " <<
                     "false or unknown.");
            return FAILING_STATE;
		  }
      }

	if ((getCommandHandleReceivedCondition()->getValue() == BooleanVariable::TRUE_VALUE())) {
		m_commandHandleVariable->setValue(getAcknowledgementValue());
	  }

	if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		return ITERATION_ENDED_STATE;
	}
      
	debugMsg("Node:getDestState",
			 "Destination from EXECUTING: no state.\n  Ancestor invariant: " 
			 << getAncestorInvariantCondition()->toString() 
			 << "\n  Invariant: " << getInvariantCondition()->toString() 
			 << "\n  End: " << getEndCondition()->toString());
	return NO_NODE_STATE;
  }

  NodeState CommandNode::getDestStateFromFailing()
  {
	checkError(isAbortCompleteConditionActive(),
			   "Abort complete for " << getNodeId().toString() << " is inactive.");

	if (getAbortCompleteCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (findVariable(Node::FAILURE_TYPE())->getValue() ==
		  FailureVariable::PARENT_FAILED()) {
		debugMsg("Node:getDestState",
				 "Destination: FINISHED.  Command node abort complete, " <<
				 "and parent failed.");
		return FINISHED_STATE;
	  }
	  else {
		debugMsg("Node:getDestState",
				 "Destination: ITERATION_ENDED.  Command node abort complete.");
		return ITERATION_ENDED_STATE;
	  }
	}

	debugMsg("Node:getDestState", "Destination: no state.");
	return NO_NODE_STATE;
  }

  //
  // Transition handlers
  //

  void CommandNode::transitionFromExecuting(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == FAILING_STATE ||
			   destState == ITERATION_ENDED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
	  if (getEndCondition()->getValue() != BooleanVariable::TRUE_VALUE())
		abort();
	}
	else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
	  if (getEndCondition()->getValue() != BooleanVariable::TRUE_VALUE())
		abort();
	}
	else if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (getPostCondition()->getValue() != BooleanVariable::TRUE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	  }
	  else
		getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
	}
	else {
	  checkError(ALWAYS_FAIL, "Should never get here.");
	}

	deactivateEndCondition();
	deactivateInvariantCondition();
	deactivateAncestorInvariantCondition();
	deactivatePostCondition();
	deactivateCommandHandleReceivedCondition();
	deactivateExecutable();
  }

  void CommandNode::transitionFromFailing(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == ITERATION_ENDED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	deactivateAbortCompleteCondition();
  }

  void CommandNode::transitionToExecuting()
  {
	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();
	activatePostCondition();
	activateCommandHandleReceivedCondition();

	setState(EXECUTING_STATE);
	execute();
  }

  void CommandNode::transitionToFailing()
  {
	activateAbortCompleteCondition();
  }

  // TODO: figure out if this should be activated on entering EXECUTING state
  void CommandNode::specializedActivateInternalVariables()
  {
	m_commandHandleVariable->activate();
  }

  void CommandNode::specializedHandleExecution()
  {
	checkError(m_command.isValid(),
			   "Node::handleExecution: Command is invalid");
	m_command->activate();
	m_command->fixValues();
	m_command->fixResourceValues();
	m_exec->enqueueCommand(m_command);
  }

  void CommandNode::abort()
  {
    if (m_command.isValid()) {
	  // Handle stupid unit test
	  if (m_exec->getExternalInterface().isId()) {
		m_exec->getExternalInterface()->invokeAbort(m_command->getName(),
													m_command->getArgValues(),
													m_conditions[abortCompleteIdx],
													m_command->m_ack);
	  }
	}
	else       
	  debugMsg("Warning", "Invalid command id in " << m_nodeId.toString());
  }

  void CommandNode::specializedDeactivateExecutable()
  {
    if (m_command.isValid())
      m_command->deactivate();
  }

  void CommandNode::specializedReset()
  {
	m_commandHandleVariable->reset();
	m_ack->reset();
  }

  void CommandNode::createCommand(const PlexilCommandBody* command) 
  {
	checkError(command->state()->nameExpr().isValid(),
			   "Attempt to create command with invalid name expression");

    PlexilStateId state = command->state();
    std::vector<ExpressionId> garbage;
	bool wasCreated = false;
    ExpressionId nameExpr = 
	  ExpressionFactory::createInstance(state->nameExpr()->name(), 
										state->nameExpr(), 
										m_connector,
										wasCreated);
	if (wasCreated)
	  garbage.push_back(nameExpr);

    LabelStr name(nameExpr->getValue());
    std::list<ExpressionId> args;
    for (std::vector<PlexilExprId>::const_iterator it = state->args().begin();
		 it != state->args().end(); 
		 ++it) {
	  ExpressionId argExpr =
		ExpressionFactory::createInstance((*it)->name(), *it, m_connector, wasCreated);
	  check_error(argExpr.isValid());
	  args.push_back(argExpr);
	  if (wasCreated)
		garbage.push_back(argExpr);
    }
    
    VariableId destVar;
    LabelStr dest_name = "";
    if (!command->dest().empty()) {
	  const PlexilExprId& destExpr = command->dest()[0]->getId();
	  dest_name = destExpr->name();
	  if (Id<PlexilVarRef>::convertable(destExpr)) {
		destVar = findVariable((Id<PlexilVarRef>) destExpr);
		// FIXME: push this check up into XML parser
		checkError(destVar.isValid(),
				   "Unknown destination variable '" << dest_name <<
				   "' in command '" << name.toString() << "' in node '" <<
				   m_nodeId.toString() << "'");
	  }
	  else if (Id<PlexilArrayElement>::convertable(destExpr)) {
		destVar = ExpressionFactory::createInstance(destExpr->name(),
													destExpr,
													m_connector);
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
		  = ExpressionFactory::createInstance(resItr->second->name(), 
											  resItr->second, 
											  m_connector,
											  wasCreated);
		check_error(resExpr.isValid());
		resourceMap[resItr->first] = resExpr;
		if (wasCreated)
		  garbage.push_back(resExpr);
	  }
	  resourceList.push_back(resourceMap);
	}

    debugMsg("Node:createCommand",
			 "Creating command '" << name.toString() << "' for node '" <<
			 m_nodeId.toString() << "'");
    m_command = (new Command(nameExpr, args, destVar, dest_name, m_ack, garbage, resourceList, getId()))->getId();
    check_error(m_command.isValid());
  }

  // Unit test variant of above
  void CommandNode::createDummyCommand() 
  {
    ExpressionId nameExpr = (new StringVariable("dummy", true))->getId();
    std::vector<ExpressionId> garbage;
	garbage.push_back(nameExpr);
    LabelStr name(nameExpr->getValue());
	// Empty arglist
    std::list<ExpressionId> args;
    
	// No destination variable
    VariableId destVar;
    LabelStr dest_name;

    // No resource
    ResourceList resourceList;
    m_command = (new Command(nameExpr, args, destVar, dest_name, m_ack, garbage, resourceList, getId()))->getId();
    check_error(m_command.isValid());
  }

  double CommandNode::getAcknowledgementValue() const 
  {
    return ((Variable*)m_ack)->getValue();
  }

  void CommandNode::printCommandHandle(std::ostream& stream, 
									   const unsigned int indent,
									   bool always) const
  {
	if (always || m_commandHandleVariable->getValue() != Expression::UNKNOWN()) {
	  std::string indentStr(indent, ' ');
	  stream << indentStr << " Command handle: " <<
		m_commandHandleVariable->toString() << '\n';
	}
  }

}
