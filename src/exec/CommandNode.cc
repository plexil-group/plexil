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
	  m_commandHandleVariable((new CommandHandleVariable())->getId())
  {
	checkError(nodeProto->nodeType() == NodeType_Command,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
			   << "\" for a CommandNode");

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
	  m_commandHandleVariable((new CommandHandleVariable())->getId())
  {
	checkError(type == COMMAND(),
			   "Invalid node type \"" << type.toString() << "\" for a CommandNode");

	// Make command handle accessible
	m_variablesByName[COMMAND_HANDLE().getKey()] = m_commandHandleVariable;

	// Construct ack variable
	// FIXME: have to delete the one created by default constructor first!
	delete (Variable*) m_ack;
	m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();

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

	// Now safe to delete command handle
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

  const LabelStr CommandNode::getCommandHandle()
  {
	if (m_commandHandleVariable.isId())
	  return m_commandHandleVariable->getValue();
	else 
	  return Expression::UNKNOWN();
  }

  // Specific behaviors for derived classes
  void CommandNode::specializedPostInit()
  {
	debugMsg("Node:postInit", "Creating command for node '" << m_nodeId.toString() << "'");
	// XML parser should have checked for this
	checkError(Id<PlexilCommandBody>::convertable(m_node->body()),
			   "Node is a command node but doesn't have a command body.");
	createCommand((PlexilCommandBody*)m_node->body());
  }

  void CommandNode::createSpecializedConditions()
  {
	// Construct command-aborted condition
	VariableId commandAbort = (new BooleanVariable())->getId();
	ExpressionListenerId abortListener = m_listeners[abortCompleteIdx];
	commandAbort->addListener(abortListener);
	m_conditions[abortCompleteIdx] = commandAbort;
	m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();
          
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


  Command::Command(const ExpressionId nameExpr, 
				   const std::list<ExpressionId>& args,
				   const VariableId dest,
                   const LabelStr& dest_name,
				   const VariableId ack,
				   const std::vector<ExpressionId>& garbage,
                   const ResourceList& resource,
				   const NodeId& parent)
    : m_id(this),
	  m_nameExpr(nameExpr),
	  m_args(args),
	  m_dest(dest),
      m_destName(dest_name),
	  m_ack(ack), 
      m_garbage(garbage),
	  m_resourceList(resource),
	  m_node(parent)
  {}

  Command::~Command() {
    for (std::vector<ExpressionId>::const_iterator it = m_garbage.begin();
		 it != m_garbage.end();
		 ++it) {
      delete (Expression*) (*it);
	}
    m_id.remove();
  }

  LabelStr Command::getName() const
  {
	return LabelStr(m_nameExpr->getValue()); 
  }

  void Command::fixValues() {
    m_argValues.clear();
    for (std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      m_argValues.push_back(expr->getValue());
    }
  }

  void Command::fixResourceValues()
  {
    m_resourceValuesList.clear();
    for(ResourceList::const_iterator resListIter = m_resourceList.begin();
		resListIter != m_resourceList.end();
		++resListIter)
      {
        ResourceValues resValues;
        for(ResourceMap::const_iterator resIter = resListIter->begin();
            resIter != resListIter->end();
			++resIter) {
		  ExpressionId expr = resIter->second;
		  check_error(expr.isValid());
		  resValues[resIter->first] = expr->getValue();
		}
        m_resourceValuesList.push_back(resValues);
      }
  }

  //more error checking here
  void Command::activate() {
	m_nameExpr->activate();
    if(m_dest != ExpressionId::noId())
      m_dest->activate();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    for (ResourceList::const_iterator resListIter = m_resourceList.begin();
		 resListIter != m_resourceList.end();
		 ++resListIter) {
	  for (ResourceMap::const_iterator resIter = resListIter->begin();
		   resIter != resListIter->end();
		   ++resIter) {
		ExpressionId expr = resIter->second;
		check_error(expr.isValid());
		expr->activate();
	  }
	}
  }

  void Command::deactivate() {
	m_nameExpr->deactivate();
    if(m_dest != ExpressionId::noId())
      m_dest->deactivate();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
  }

  const std::string& Command::getDestName() const {
    return m_destName.toString();
  }

}
