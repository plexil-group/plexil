/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

//
// *** TODO ***
// Resolve use of m_garbage with respect to conditions

#include "Node.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "Expressions.hh"
#include "ExpressionFactory.hh"
#include "ExternalInterface.hh"
#include "NodeStateManager.hh"
#include "PlexilExec.hh"
#include "Variables.hh"
#include "XMLUtils.hh"

#include <algorithm> // for find_if
#include <vector>
#include <sstream>
#include <iomanip> // for setprecision

namespace PLEXIL {
  const std::vector<double>& Node::ALL_CONDITIONS() {
    static std::vector<double>* sl_allConds = NULL;
    if (sl_allConds == NULL)
      {
	// *** N.B.: Order MUST agree with enum conditionIndex!
	sl_allConds = new std::vector<double>();
	sl_allConds->push_back(SKIP_CONDITION());
	sl_allConds->push_back(START_CONDITION());
	sl_allConds->push_back(END_CONDITION());
	sl_allConds->push_back(INVARIANT_CONDITION());
	sl_allConds->push_back(PRE_CONDITION());
	sl_allConds->push_back(POST_CONDITION());
	sl_allConds->push_back(REPEAT_CONDITION());
	sl_allConds->push_back(ANCESTOR_INVARIANT_CONDITION());
	sl_allConds->push_back(ANCESTOR_END_CONDITION());
	sl_allConds->push_back(PARENT_EXECUTING_CONDITION());
	sl_allConds->push_back(CHILDREN_WAITING_OR_FINISHED());
	sl_allConds->push_back(ABORT_COMPLETE());
	sl_allConds->push_back(PARENT_WAITING_CONDITION());
	sl_allConds->push_back(PARENT_FINISHED_CONDITION());
	sl_allConds->push_back(COMMAND_HANDLE_RECEIVED_CONDITION());
	// inexpensive sanity check
	assertTrue(sl_allConds->size() == conditionIndexMax,
		   "INTERNAL ERROR: Inconsistency between conditionIndex enum and ALL_CONDITIONS");
      }
    return *sl_allConds;
  }

  unsigned int Node::getConditionIndex(const LabelStr& cName) {
    double nameKey = cName.getKey();
    const std::vector<double>& allConds = ALL_CONDITIONS();
    for (size_t i = 0; i < conditionIndexMax; i++) {
      if (allConds[i] == nameKey)
	return i;
    }
    assertTrueMsg(ALWAYS_FAIL,
		  cName.toString() << " is not a valid condition name");
  }

  LabelStr Node::getConditionName(unsigned int idx)
  {
    return LabelStr(ALL_CONDITIONS()[idx]);
  }

  class ConditionChangeListener : public ExpressionListener {
  public:
    ConditionChangeListener(NodeId node, const LabelStr& cond)
      : ExpressionListener(), m_node(node), m_cond(cond) {}
    void notifyValueChanged(const ExpressionId& /* expression */) {
      debugMsg("Node:conditionChange",
	       m_cond.toString() << " may have changed value in " <<
	       m_node->getNodeId().toString());

      m_node->conditionChanged();
    }
  protected:
  private:
    NodeId m_node;
    const LabelStr& m_cond;
  };

  class RealNodeConnector : public NodeConnector {
  public:
    RealNodeConnector(const NodeId& node) 
      : NodeConnector(), m_node(node) 
    {
    }

    const VariableId& findVariable(const PlexilVarRef* ref)
    {
      return m_node->findVariable(ref);
    }

    const VariableId& findVariable(const LabelStr& name, bool recursive = false)
    {
      return m_node->findVariable(name, recursive);
    }

    const ExecConnectorId& getExec()
    {
      return m_node->getExec();
    }

    const NodeId& getNode() const
    {
      return m_node;
    }

  private:
    NodeId m_node;
  };

  const LabelStr& 
  Node::nodeTypeToLabelStr(PlexilNodeType nodeType)
  {
    switch(nodeType) {
      case NodeType_NodeList:
        return LIST();

      case NodeType_Command:
        return COMMAND();

      case NodeType_Assignment:
        return ASSIGNMENT();

      case NodeType_Update:
        return UPDATE();

      case NodeType_Request:
        return REQUEST();

      case NodeType_Empty:
        return EMPTY();

      case NodeType_LibraryNodeCall:
        return LIBRARYNODECALL();

        // fall thru case
      default:
        checkError(ALWAYS_FAIL,
                   "Invalid node type " << nodeType);
        break;
      }
    return EMPTY_LABEL();
  }

  Node::Node(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent)
    : m_id(this),
	  m_parent(parent),
	  m_exec(exec),
      m_connector((new RealNodeConnector(m_id))->getId()),
	  m_node(node),
	  m_nodeId(node->nodeId()),
	  m_nodeType(nodeTypeToLabelStr(node->nodeType())), // Can throw exception
	  m_sortedVariableNames(new std::vector<double>()),
      m_priority(node->priority()),
      m_state(INACTIVE_STATE),
	  m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false),
	  m_cleanedConditions(false),
	  m_cleanedVars(false),
      m_transitioning(false),
	  m_checkConditionsPending(false)
  {
	debugMsg("Node:node", "Creating node \"" << node->nodeId() << "\"");

	m_stateManager = NodeStateManager::getStateManager(m_nodeType);
	commonInit();
	setConditionDefaults();

	// Instantiate declared variables
	createDeclaredVars(node->declarations());

	// get interface variables
	getVarsFromInterface(node->interface());

	if (m_nodeType == LIST()) {
	  // Instantiate child nodes, if any
	  debugMsg("Node:node", "Creating child nodes.");
	  // XML parser should have checked for this
	  checkError(Id<PlexilListBody>::convertable(node->body()),
				 "Node " << m_nodeId.toString() << " is a list node but doesn't have a " <<
				 "list body.");
	  createChildNodes((PlexilListBody*) node->body());
	}

	else if (m_nodeType == LIBRARYNODECALL()) {
	  // Create library call node
	  debugMsg("Node:node", "Creating library node call.");
	  // XML parser should have checked for this
	  checkError(Id<PlexilLibNodeCallBody>::convertable(node->body()),
				 "Node " << m_nodeId.toString() << " is a library node call but doesn't have a " <<
				 "library node call body.");
	  createLibraryNode(node);
	}
  }

  // Used only by module test
  Node::Node(const LabelStr& type, const LabelStr& name, const NodeState state,
			 const bool skip, const bool start, const bool pre, const bool invariant, const bool post,
			 const bool end, const bool repeat, const bool ancestorInvariant,
			 const bool ancestorEnd, const bool parentExecuting, const bool childrenFinished,
			 const bool commandAbort, const bool parentWaiting, 
			 const bool parentFinished, const bool cmdHdlRcvdCondition, const ExecConnectorId& exec)
    : m_id(this),
	  m_parent(NodeId::noId()),
	  m_exec(exec),
	  m_node(PlexilNodeId::noId()),
	  m_nodeId(name),
	  m_nodeType(type),
	  m_sortedVariableNames(new std::vector<double>()),
      m_state(state),
	  m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false), 
	  m_cleanedConditions(false), 
	  m_cleanedVars(false),
      m_transitioning(false), 
	  m_checkConditionsPending(false)
  {
    m_stateManager = NodeStateManager::getStateManager(m_nodeType);
    commonInit();
	activateInternalVariables();

	// N.B.: Must be same order as ALL_CONDITIONS() and conditionIndex enum!
    bool values[conditionIndexMax] =
	  {skip, start, end, invariant, pre, post, repeat, ancestorInvariant,
	   ancestorEnd, parentExecuting, childrenFinished, commandAbort,
	   parentWaiting, parentFinished, cmdHdlRcvdCondition};
    for (unsigned int i = 0; i < conditionIndexMax; i++) {
      debugMsg("Node:node",
			   "Creating internal variable " << LabelStr(ALL_CONDITIONS()[i]).toString() <<
			   " with value " << values[i] << " for node " << m_nodeId.toString());
      ExpressionId expr = (new BooleanVariable((double) values[i]))->getId();
      m_conditions[i] = expr;
      expr->addListener(m_listeners[i]);
      m_garbageConditions.insert(i);
    }
    if (m_nodeType == COMMAND())
      m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();
    else
      m_ack = (new BooleanVariable(BooleanVariable::UNKNOWN()))->getId();
  }

  // FIXME: Doesn't seem to delete local variables, command, update, or assignment
  Node::~Node() {
	delete m_sortedVariableNames;
    delete (RealNodeConnector*) m_connector;
    cleanUpConditions();
    cleanUpVars();

    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }

    m_id.remove();
  }

  void Node::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

	debugMsg("Node:cleanUpConditions", " for " << m_nodeId.toString());

    // Clean up condition listeners
    for (unsigned int i = 0; i < conditionIndexMax; i++) {
      if (m_listeners[i].isId()) {
		debugMsg("Node:cleanUpConds",
				 "<" << m_nodeId.toString() << "> Removing condition listener for " <<
				 getConditionName(i).toString());
		m_conditions[i]->removeListener(m_listeners[i]);
		delete (ExpressionListener*) m_listeners[i];
		m_listeners[i] = ExpressionListenerId::noId();
      }
    }
 
    // Clean up children
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();

    // Clean up conditions
    for (std::set<unsigned int>::iterator it = m_garbageConditions.begin();
		 it != m_garbageConditions.end();
		 ++it) {
      debugMsg("Node:cleanUpConds",
			   "<" << m_nodeId.toString() << "> Removing condition " << getConditionName(*it).toString());
      delete (Expression*) m_conditions[*it];
      m_conditions[*it] = ExpressionId::noId();
    }

    if(m_assignment.isValid()) {
      debugMsg("Node:cleanUpConds", "<" << m_nodeId.toString() << "> Removing assignment.");
      delete (Assignment*) m_assignment;
    }
    if(m_command.isValid()) {
      debugMsg("Node:cleanUpConds", "<" << m_nodeId.toString() << "> Removing command.");
      delete (Command*) m_command;
    }
    if(m_update.isValid()) {
      debugMsg("Node:cleanUpConds", "<" << m_nodeId.toString() << "> Removing update.");
      delete (Update*) m_update;
    }

    m_cleanedConditions = true;
  }

  void Node::cleanUpVars() {
    if (m_cleanedVars)
      return;
    checkError(m_cleanedConditions,
			   "Have to clean up variables before conditions can be cleaned.");

	debugMsg("Node:cleanUpVars", " for " << m_nodeId.toString());

	// Delete user-spec'd variables
    for (std::set<double>::iterator it = m_garbage.begin(); it != m_garbage.end(); ++it) {
      if (m_variablesByName.find(*it) != m_variablesByName.end()) {
		debugMsg("Node:cleanUpVars",
				 "<" << m_nodeId.toString() << "> Removing " << LabelStr(*it).toString());
		delete (Variable*) m_variablesByName.find(*it)->second;
		m_variablesByName.erase(*it);
      }
    }

	// Clear map
	m_variablesByName.clear();

	// Delete timepoint variables
	for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
	  delete (Variable*) m_startTimepoints[s];
	  delete (Variable*) m_endTimepoints[s];
	  m_startTimepoints[s] = m_endTimepoints[s] = VariableId::noId();
	}

	// Delete internal variables
	if (m_commandHandleVariable.isId()) {
	  delete (Variable*) m_commandHandleVariable;
	  m_commandHandleVariable = VariableId::noId();
	}
    if (m_ack.isId()) {
      delete (Variable*) m_ack;
	  m_ack = VariableId::noId();
	}
	delete (Variable*) m_outcomeVariable;
	m_outcomeVariable = VariableId::noId();
	delete (Variable*) m_failureTypeVariable;
	m_failureTypeVariable = VariableId::noId();
	delete (Variable*) m_stateVariable;
	m_stateVariable = VariableId::noId();

    m_cleanedVars = true;
  }

  void Node::commonInit() {
    debugMsg("Node:node", "Instantiating internal variables...");
    // Instantiate state/outcome/failure variables
	// The contortions with getKey() are an attempt to minimize LabelStr copying
    m_variablesByName[STATE().getKey()] = m_stateVariable = (new StateVariable())->getId();
    ((StateVariable*) m_stateVariable)->setNodeState(m_state);

	m_variablesByName[OUTCOME().getKey()] = m_outcomeVariable = (new OutcomeVariable())->getId();
    m_variablesByName[FAILURE_TYPE().getKey()] = m_failureTypeVariable = (new FailureVariable())->getId();
	if (m_nodeType == COMMAND()) {
	  m_variablesByName[COMMAND_HANDLE().getKey()] = m_commandHandleVariable = (new CommandHandleVariable())->getId();
	}

    //instantiate timepoint variables
    debugMsg("Node:node", "Instantiating timepoint variables.");
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      ExpressionId stp = (new RealVariable())->getId();
      double stpName = START_TIMEPOINT_NAMES()[s];
      m_startTimepoints[s] = m_variablesByName[stpName] = stp;

      ExpressionId etp = (new RealVariable())->getId();
      const LabelStr& etpName = END_TIMEPOINT_NAMES()[s];
      m_endTimepoints[s] = m_variablesByName[etpName] = etp;
    }

	// construct condition listeners (but not conditions)
	for (int i = 0; i < conditionIndexMax; i++) {
	  m_listeners[i] = 
		(new ConditionChangeListener(m_id, ALL_CONDITIONS()[i]))->getId();
	}
  }

  // Use existing Boolean constants for the condition defaults
  void Node::setConditionDefaults() {
	// These may be user-specified
    m_conditions[skipIdx] = BooleanVariable::FALSE_EXP();
    m_conditions[startIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[endIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[invariantIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[preIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[postIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[repeatIdx] = BooleanVariable::FALSE_EXP();

	// These will be overridden in any non-root node,
	// but they depend on user-specified conditions,
	// so do these in createConditions() below.
    m_conditions[ancestorInvariantIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[ancestorEndIdx] = BooleanVariable::FALSE_EXP();

	if (m_parent.isId()) {
	  // These conditions only depend on the node state variable,
	  // which is already initialized.
	  ExpressionId parentExecuting =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::EXECUTING_EXP()))->getId();
	  ExpressionListenerId parentExecutingListener = m_listeners[parentExecutingIdx];
	  parentExecuting->addListener(parentExecutingListener);
	  m_conditions[parentExecutingIdx] = parentExecuting;
	  m_garbageConditions.insert(parentExecutingIdx);

	  ExpressionId parentWaiting =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::WAITING_EXP()))->getId();
	  ExpressionListenerId parentWaitingListener = m_listeners[parentWaitingIdx];
	  parentWaiting->addListener(parentWaitingListener);
	  m_conditions[parentWaitingIdx] = parentWaiting;
	  m_garbageConditions.insert(parentWaitingIdx);

	  ExpressionId parentFinished =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::FINISHED_EXP()))->getId();
	  ExpressionListenerId parentFinishedListener = m_listeners[parentFinishedIdx];
	  parentFinished->addListener(parentFinishedListener);
	  m_conditions[parentFinishedIdx] = parentFinished;
	  m_garbageConditions.insert(parentFinishedIdx);
	}
	else {
	  // Dummies for root node
	  m_conditions[parentExecutingIdx] = BooleanVariable::TRUE_EXP();
	  m_conditions[parentWaitingIdx] = BooleanVariable::FALSE_EXP();
	  m_conditions[parentFinishedIdx] = BooleanVariable::FALSE_EXP();
	}

	// This will be overridden in any node with children (List or LibraryNodeCall)
    m_conditions[childrenWaitingOrFinishedIdx] = BooleanVariable::UNKNOWN_EXP();

	// This will be overridden in Command, Update, Request nodes
    m_conditions[abortCompleteIdx] = BooleanVariable::UNKNOWN_EXP();

	// This will be overridden in Command nodes
    m_conditions[commandHandleReceivedIdx] = BooleanVariable::TRUE_EXP();
  }

  void Node::postInit() {
    checkError(!m_postInitCalled, "Called postInit on node '" << m_nodeId.toString() << "' twice.");
    m_postInitCalled = true;

    debugMsg("Node:postInit", "Creating conditions for node '" << m_nodeId.toString() << "'");
    //create conditions and listeners
    createConditions(m_node->conditions());

    //create assignment/command
    if(m_nodeType == COMMAND()) {
      debugMsg("Node:postInit", "Creating command for node '" << m_nodeId.toString() << "'");
	  // XML parser should have checked for this
      checkError(Id<PlexilCommandBody>::convertable(m_node->body()),
		 "Node is a command node but doesn't have a command body.");
      createCommand((PlexilCommandBody*)m_node->body());
    }
    else if(m_nodeType == ASSIGNMENT()) {
      debugMsg("Node:postInit",
	       "Creating assignment for node '" << m_nodeId.toString() << "'");
	  // XML parser should have checked for this
      checkError(Id<PlexilAssignmentBody>::convertable(m_node->body()),
		 "Node is an assignment node but doesn't have an assignment body.");
      createAssignment((PlexilAssignmentBody*)m_node->body());
    }
    else if (m_nodeType == UPDATE()) {
      debugMsg("Node:postInit", "Creating update for node '" << m_nodeId.toString() << "'");
	  // XML parser should have checked for this
      checkError(Id<PlexilUpdateBody>::convertable(m_node->body()),
		 "Node is an update node but doesn't have an update body.");
      createUpdate((PlexilUpdateBody*)m_node->body());
    }

    //call postInit on all children
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->postInit();
  }


  void Node::createAssignment(const PlexilAssignmentBody* body) 
  {
    //we still only support one variable on the LHS
	// FIXME: push this check up into XML parser
    checkError(body->dest().size() >= 1,
	       "Need at least one destination variable in assignment.");
    const PlexilExprId& destExpr = (body->dest())[0]->getId();
    VariableId dest;
    LabelStr destName;
    bool deleteLhs = false;
    if (Id<PlexilVarRef>::convertable(destExpr)) {
      destName = destExpr->name();
      dest = findVariable((Id<PlexilVarRef>) destExpr);
	  // FIXME: push this check up into XML parser
      checkError(dest.isValid(),
                 "Dest variable '" << destName <<
                 "' not found in assignment node '" << m_nodeId.toString() << "'");
    }
    else if (Id<PlexilArrayElement>::convertable(destExpr)) {
      dest =
		(VariableId)
		ExpressionFactory::createInstance(destExpr->name(),
										  destExpr,
										  m_connector);
      // *** beef this up later ***
	  PlexilArrayElement* arrayElement = (PlexilArrayElement*) destExpr;
	  debugMsg("ArrayElement:ArrayElement", " name = " << arrayElement->getArrayName() << ". To: " << dest->toString());
	  int e_index = dest->toString().find(": ", dest->toString().length()-15);
	  int b_index = dest->toString().find("u]", dest->toString().length()-40) + 2;
	  int diff_index = e_index - b_index;
	  std::string m_index = " ";
	  if(e_index != std::string::npos)
	  {

		  m_index = dest->toString().substr(e_index-diff_index,diff_index);
	  }
	  debugMsg("ArrayElement:ArrayElement", " b_index = " << b_index << ". e_index = " << e_index << ". diff_index" << diff_index);
	  const std::string m_str = std::string("").append(arrayElement->getArrayName()).append(m_index);
	  destName = LabelStr(m_str);
      deleteLhs = true;
    }
    else {
	  // FIXME: push this check up into XML parser 
	  checkError(ALWAYS_FAIL, "Invalid left-hand side to an assignment");
    }

    bool deleteRhs = false;
    ExpressionId rhs =
      ExpressionFactory::createInstance(body->RHS()->name(), 
										body->RHS(),
										m_connector,
										deleteRhs);
    m_assignment =
      (new Assignment(dest, rhs, m_ack, destName, deleteLhs, deleteRhs))->getId();
  }

  void Node::createCommand(const PlexilCommandBody* command) 
  {
	checkError(command->state()->nameExpr().isValid(),
			   "Attempt to create command with invalid name expression");

    PlexilStateId state = command->state();
    std::vector<ExpressionId> garbage;
	bool nameIsGarbage = false;
    ExpressionId nameExpr = 
	  ExpressionFactory::createInstance(state->nameExpr()->name(), 
										state->nameExpr(), 
										m_connector,
										nameIsGarbage);
    LabelStr name(nameExpr->getValue());
    std::list<ExpressionId> args;
    for(std::vector<PlexilExprId>::const_iterator it = state->args().begin();
	it != state->args().end(); ++it) 
    {
       ExpressionId argExpr;
       if(Id<PlexilVarRef>::convertable(*it)) 
       {
          argExpr = findVariable(*it);
		  // FIXME: push this check up into XML parser
          checkError(argExpr.isValid(),
                     "Unknown variable '" << (*it)->name() <<
                     "' in argument list for command '" << nameExpr->getValue() <<
                     "' in node '" << m_nodeId.toString() << "'");
       }
       else 
       {
          argExpr = ExpressionFactory::createInstance((*it)->name(), *it, m_connector);
          garbage.push_back(argExpr);
          check_error(argExpr.isValid());
       }
       args.push_back(argExpr);
    }
    
    VariableId destVar;
    LabelStr dest_name = "";
    if (!command->dest().empty()) {
        const PlexilExprId& destExpr = command->dest()[0]->getId();
        dest_name = destExpr->name();
        if (Id<PlexilVarRef>::convertable(destExpr))
          {
            destVar = findVariable((Id<PlexilVarRef>) destExpr);
			// FIXME: push this check up into XML parser
            checkError(destVar.isValid(),
                       "Unknown destination variable '" << dest_name <<
                       "' in command '" << name.toString() << "' in node '" <<
                       m_nodeId.toString() << "'");
          }
        else if (Id<PlexilArrayElement>::convertable(destExpr))
          {
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
		ExpressionId resExpr;
		if (Id<PlexilVarRef>::convertable(resItr->second)) {
		  resExpr = findVariable(resItr->second);
		  // FIXME: push this check up into XML parser
		  checkError(resExpr.isValid(),
					 "Unknown variable '" << resItr->second->name() <<
					 "' in resource list for command '" << nameExpr->getValue() <<
					 "' in node '" << m_nodeId.toString() << "'");
		}
		else {
		  resExpr = ExpressionFactory::createInstance(resItr->second->name(), 
													  resItr->second, m_connector);
		  check_error(resExpr.isValid());
		  garbage.push_back(resExpr);
		}
		resourceMap[resItr->first] = resExpr;
	  }
	  resourceList.push_back(resourceMap);
	}

    debugMsg("Node:createCommand",
	     "Creating command '" << name.toString() << "' for node '" <<
	     m_nodeId.toString() << "'");
    m_command = (new Command(nameExpr, args, destVar, dest_name, m_ack, garbage, resourceList, getId()))->getId();
    check_error(m_command.isValid());
  }

  void Node::createUpdate(const PlexilUpdateBody* body) 
  {
    PlexilUpdateId update = body->update();
    ExpressionMap updatePairs;
    std::list<ExpressionId> garbage;

    if(update.isValid()) {
      for(std::vector<std::pair<std::string, PlexilExprId> >::const_iterator it =
			update->pairs().begin(); it != update->pairs().end(); ++it) {
		LabelStr nameStr(it->first);
		PlexilExprId foo = it->second;
		debugMsg("Node:createUpdate", "Adding pair '" << nameStr.toString());
		ExpressionId valueExpr = ExpressionId::noId();

		if(Id<PlexilVarRef>::convertable(foo)) {
		  valueExpr = findVariable(foo);
		  // FIXME: push this check up into XML parser
		  checkError(valueExpr.isValid(),
					 "Unknown variable " << foo->name() << " in update for node " <<
					 m_nodeId.toString());
		}      
		else {
		  valueExpr =
			ExpressionFactory::createInstance(foo->name(), foo, m_connector);
		  garbage.push_back(valueExpr);
		  check_error(valueExpr.isValid());
		}
		updatePairs.insert(std::make_pair((double) nameStr, valueExpr));
      }
    }

    m_update = (new Update(m_id, updatePairs, m_ack, garbage))->getId();
  }

  // Make the node (and its children, if any) active.
  void Node::activate()
  {
	// Activate internal variables
	activateInternalVariables();

	// These are the only conditions we care about in the INACTIVE state.
	// See DefaultStateManager.cc, specifically DefaultInactiveStateComputer::getDestState().
	m_conditions[parentExecutingIdx]->activate();
    m_listeners[parentExecutingIdx]->activate();
	m_conditions[parentFinishedIdx]->activate();
    m_listeners[parentFinishedIdx]->activate();
	
    // Activate all children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->activate();
  }

  void Node::activateInternalVariables()
  {
	// Activate internal variables
    m_stateVariable->activate();
	// TODO: figure out if these should be activated on entering EXECUTING state
    m_outcomeVariable->activate();
    m_failureTypeVariable->activate();
	if (m_commandHandleVariable.isId())
	  m_commandHandleVariable->activate();

	// Activate timepoints
	// TODO: figure out if they should be inactive until entering the corresponding state
	for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
	  m_startTimepoints[s]->activate();
	  m_endTimepoints[s]->activate();
	}
  }

  ExpressionId& Node::getCondition(const LabelStr& name) {
    return m_conditions[getConditionIndex(name)];
  }

  double Node::getAcknowledgementValue() const 
  {
    return ((Variable*)m_ack)->getValue();
  }

  void Node::createConditions(const std::map<std::string, PlexilExprId>& conds) 
  {
	if(m_parent.isId()) {
	  ExpressionId ancestorInvariant =
		(new Conjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_INVARIANT_CONDITION()),
												 m_connector))->getId(),
						 true,
						 (new TransparentWrapper(m_parent->getCondition(INVARIANT_CONDITION()),
												 m_connector))->getId(),
						 true))->getId();
	  ExpressionListenerId ancestorInvariantListener = m_listeners[ancestorInvariantIdx];
	  // Replace default ancestor invariant condition with real one
	  ancestorInvariant->addListener(ancestorInvariantListener);
	  m_conditions[ancestorInvariantIdx] = ancestorInvariant;
	  m_garbageConditions.insert(ancestorInvariantIdx);

	  ExpressionId ancestorEnd =
		(new Disjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_END_CONDITION()),
												 m_connector))->getId(),
						 true,
						 (new TransparentWrapper(m_parent->getCondition(END_CONDITION()),
												 m_connector))->getId(),
						 true))->getId();
	  ExpressionListenerId ancestorEndListener = m_listeners[ancestorEndIdx];
	  // Replace default ancestor end condition with real one
	  ancestorEnd->addListener(ancestorEndListener);
	  m_conditions[ancestorEndIdx] = ancestorEnd;
	  m_garbageConditions.insert(ancestorEndIdx);

	}

	// Add user-specified conditions
	for (std::map<std::string, PlexilExprId>::const_iterator it = conds.begin(); 
		 it != conds.end(); 
		 ++it) {
	  const LabelStr condName(it->first);
	  unsigned int condIdx = getConditionIndex(condName);

	  // Delete existing condition if required
	  // (e.g. explicit override of default end condition for list or library call node)
	  if (m_garbageConditions.find(condIdx) != m_garbageConditions.end()) {
		m_conditions[condIdx]->removeListener(m_listeners[condIdx]);
		delete (Expression*) m_conditions[condIdx];
	  }
	  ExpressionId expr = ExpressionId::noId();
	  if(Id<PlexilVarRef>::convertable(it->second)) {
		expr = findVariable(it->second);
	  }
	  else {
		expr = ExpressionFactory::createInstance(it->second->name(), 
												 it->second,
												 m_connector);
		m_garbageConditions.insert(condIdx);
	  }
	  ExpressionListenerId condListener = m_listeners[condIdx];
	  m_conditions[condIdx] = expr;
	  expr->addListener(condListener);
	}

	if (m_nodeType == COMMAND()) {
	  // Construct command-aborted condition
	  VariableId commandAbort = (new BooleanVariable())->getId();
	  ExpressionListenerId abortListener = m_listeners[abortCompleteIdx];
	  commandAbort->addListener(abortListener);
	  m_conditions[abortCompleteIdx] = commandAbort;
	  m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();
          
	  // Construct real end condition
	  m_conditions[endIdx]->removeListener(m_listeners[endIdx]);
	  ExpressionId interruptEndCond = (new InterruptibleCommandHandleValues(m_ack))->getId();
	  ExpressionId conjunctCondition = (new Conjunction((new IsKnown(m_ack))->getId(),
														true, 
														m_conditions[endIdx],
														m_garbageConditions.find(endIdx) != m_garbageConditions.end()))->getId();
	  ExpressionId realEndCondition =
		(new Disjunction(interruptEndCond, true, conjunctCondition, true))->getId();
	  realEndCondition->addListener(m_listeners[endIdx]);
	  m_conditions[endIdx] = realEndCondition;
	  m_garbageConditions.insert(endIdx);
          
	  // Listen to any change in the command handle so that the internal variable 
	  // CommandHandleVariable can be updated
	  ExpressionId commandHandleCondition = (new AllCommandHandleValues(m_ack))->getId();
	  commandHandleCondition->ignoreCachedValue();
	  commandHandleCondition->addListener(m_listeners[commandHandleReceivedIdx]);
	  m_conditions[commandHandleReceivedIdx] = commandHandleCondition;
	  m_garbageConditions.insert(commandHandleReceivedIdx);
	}
	else if (m_nodeType == ASSIGNMENT() || m_nodeType == UPDATE()) {
	  // Construct real end condition
	  m_conditions[endIdx]->removeListener(m_listeners[endIdx]);
	  m_ack = (new BooleanVariable(BooleanVariable::UNKNOWN()))->getId();
	  ExpressionId realEndCondition =
		(new Conjunction(m_ack,
						 false, 
						 m_conditions[endIdx],
						 m_garbageConditions.find(endIdx) != m_garbageConditions.end()))->getId();
	  realEndCondition->addListener(m_listeners[endIdx]);
	  m_conditions[endIdx] = realEndCondition;
	  m_garbageConditions.insert(endIdx);
	}
  }

  void Node::createChildNodes(const PlexilListBody* body) 
  {
    // checkError(m_nodeType == LIST(), "Attempted to create child nodes for a non-list node.");
	try {
	  for (std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
		   it != body->children().end(); 
		   ++it)
		m_children.push_back((new Node(*it, m_exec, m_id))->getId());
	}
	catch (const Error& e) {
	  debugMsg("Node:node", " Error creating child nodes: " << e);
	  // Clean up 
	  while (!m_children.empty()) {
		delete (Node*) m_children.back();
		m_children.pop_back();
	  }
	  // Rethrow so that outer error handler can deal with this as well
	  throw;
	}

    ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
    ExpressionListenerId listener = m_listeners[childrenWaitingOrFinishedIdx];
    cond->addListener(listener);
    m_conditions[childrenWaitingOrFinishedIdx] = cond;
    m_garbageConditions.insert(childrenWaitingOrFinishedIdx);

    ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
    listener = m_listeners[endIdx];
    endCond->addListener(listener);
    m_conditions[endIdx] = endCond;
    m_garbageConditions.insert(endIdx);
  }

  // Check aliases against interfaceVars.
  // Remove all that are found from aliases.
  // If a variable exists in interfaceVars but not aliases:
  //  - and it has a default value, generate the variable with the default value;
  //  - and it doesn't have a default value, signal an error.
  // libNode is only used for error message generation.

   void Node::testLibraryNodeParameters(const PlexilNodeId& libNode, 
										const std::vector<PlexilVarRef*>& interfaceVars,
										PlexilAliasMap& aliases)
   {
	 // check each variable in the interface to ensure it is
	 // referenced in the alias list
      
	 for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
		  var != interfaceVars.end(); ++var) {
	   // get var label and matching value in alias list
	   LabelStr varLabel((*var)->name());
	   PlexilExprId& aliasValue = aliases[varLabel];

	   // check that variable referenced in alias list         
	   // if it is missing then check whether formal parameter has a default value
	   if (!aliasValue.isId()) {
		 if ((*var)->defaultValue().isId()) {
		   // check that the default value is valid
		   checkError(Id<PlexilValue>::convertable((*var)->defaultValue()), 
					  "Expected PlexilValue.");
		   const Id<PlexilValue>& defaultValue = 
			 (Id<PlexilValue>&)(*var)->defaultValue();
		   checkError(defaultValue->value() != "UNKNOWN",
					  "Interface variable '" << 
					  varLabel.toString() <<
					  "' in library node '" << libNode->nodeId() << 
					  "' missing in call from '" << getNodeId().toString() <<
					  "' and no default is specified");
          
		   // construct variable with default value
		   debugMsg("Node:testLibraryNodeParameters", 
					"Constructing defaulted interface variable " << varLabel.c_str());
		   VariableId newVar =
			 (VariableId)
			 ExpressionFactory::createInstance(PlexilParser::valueTypeString((*var)->type()) + "Value",
											   defaultValue->getId(),
											   m_connector);
		   m_variablesByName[varLabel] = newVar;
		   m_localVariables.push_back(newVar);
		 }
		 else {
		   // no default value provided
		   checkError(ALWAYS_FAIL,
					  "Interface variable '" << 
					  varLabel.toString() <<
					  "' in library node '" << libNode->nodeId() << 
					  "' missing in call from '" << getNodeId().toString() <<
					  "' and no default is specified");
		 }
	   }

	   // remove value for alias copy for later checking
	   aliases.erase(varLabel);
	 }
   }

   void Node::createLibraryNode(const PlexilNodeId& node)
   {
      checkError(m_nodeType == LIBRARYNODECALL(),
                 "Attempted to create library node in non-library node call in: " 
                 << getNodeId().toString());
      
      // get node body
      const PlexilLibNodeCallBody* body = (PlexilLibNodeCallBody*)node->body();
      
      // get the lib node and its interface
      const PlexilNodeId& libNode = body->libNode();
      const PlexilInterfaceId& libInterface = libNode->interface();
      
      // if there is no interface, there must be no variables
      if (libInterface.isNoId()) {
         checkError(body->aliases().size() == 0,
                    "Variable aliases in '" << getNodeId().toString() <<
                    "' do not match interface in '" << 
                    libNode->nodeId() << "'");
      }
      
      // otherwise check variables in interface
      else {
         // make a copy of the alias map
         PlexilAliasMap aliasesCopy(body->aliases());

         // check whether all "In" variables in the interface are referenced in the alias list
         testLibraryNodeParameters(libNode, libInterface->in(), aliasesCopy);

         // check whether all "InOut" variables in the interface are referenced in the alias list
         testLibraryNodeParameters(libNode, libInterface->inOut(), aliasesCopy);

         // check that every veriable in alias list has been referenced
         // or has a default value
         checkError(aliasesCopy.size() == 0, "Unknown variable '"
                    << LabelStr(aliasesCopy.begin()->first).toString() 
                    << "' passed in call to '" << libNode->nodeId() << "' from '"
                    << getNodeId().toString() << "'");
      }

      // link aliases to variables or values
      for (PlexilAliasMap::const_iterator alias = body->aliases().begin();
           alias != body->aliases().end();
		   ++alias) {
         LabelStr paramName(alias->first);
         
         // if this is a variable reference, look it up
         if (Id<PlexilVarRef>::convertable(alias->second)) {
            const PlexilVarRef* paramVar = alias->second;
            
            // find variable in interface
            const PlexilVarRef* iVar = libInterface->findVar(paramName.toString());
            checkError(iVar != NULL,
                       "Variable '" << paramName.toString()
                       << "' referenced in '" << getNodeId().toString()
                       << "' does not appear in interface of '" 
                       << libNode->nodeId() << "' ");
            
            // check type
            checkError(iVar->type() == paramVar->type(), 
                       "Variable type mismatch between formal parameter '" 
                       << iVar->name() << "' (" << iVar->type() << ") and actual variable '" 
                       << paramVar->name() << "' (" << paramVar->type() << ") "
                       << "' referenced in '" << getNodeId().toString() << "'");

            // find the expression form
            VariableId var = findVariable(alias->second);
			// FIXME: push this check up into XML parser
            checkError(var.isId(), "Unknown variable '" 
                       << alias->second->name()
                       << "' referenced in call to '" << libNode->nodeId() << "' from '"
                       << getNodeId().toString() << "'");
			// FIXME: push this check up into XML parser
            checkError(Id<VariableImpl>::convertable(var)
					   || Id<AliasVariable>::convertable(var),
					   "Expression not a variable '" 
                       << alias->second->name()
                       << "' referenced in call to '" << libNode->nodeId() << "' from '"
                       << getNodeId().toString() << "'");

			if (iVar->type() == ARRAY) {
			  // check for array element type match
			  // iVar should have variable definition
			  const PlexilArrayVarId ivarDef = (const PlexilArrayVarId&) iVar->variable();
			  checkError(ivarDef.isId(), 
						 "Internal error: interface array variable '" << paramName.c_str()
						 << "' is missing its variable definition");
			  const Id<ArrayVariable> arrayVar = (const Id<ArrayVariable>&) var;
			  checkError(arrayVar.isId(),
						 "Internal error: variable '" << alias->second->name()
						 << "' doesn't seem to be an array variable");
			  checkError(arrayVar->getElementType() == ivarDef->elementType(),
						 "Array variable type mismatch between formal parameter '"
						 << iVar->name() << "' (" << ivarDef->elementType() 
						 << ") and actual variable '" << paramVar->name()
						 << "' (" << arrayVar->getElementType()
						 << ") referenced in '" << getNodeId().toString() << "'");
			}

            // check that read only variables appear in the the In interface
            checkError(!var->isConst() || libInterface->findInVar(paramName.toString()),
                       "Constant variable '" << alias->second->name()
                       << "' referenced in '" << getNodeId().toString() 
                       << "' is aliased to '" <<  iVar->name()
                       << "' declared as InOut in '" << libNode->nodeId() << "'");

            // add this variable to node
			debugMsg("Node:createLibraryNode",
					 " aliasing parameter variable " << paramName.c_str() << " to actual variable " << paramVar->name());
            m_variablesByName[paramName] = var;
         }
         
		 // if this is a value, create a local variable for it
         else if (Id<PlexilValue>::convertable(alias->second))
         {
            Id<PlexilValue> value = (Id<PlexilValue>)alias->second;
            debugMsg("Node:createLibraryNode",
                     " Constructing variable for " << value->name()
                     << " literal with value " << value->value() 
                     << " as library node interface variable " << paramName.c_str());
            VariableId varId = 
			  (VariableId)
              ExpressionFactory::createInstance(value->name(),
                                                value->getId(),
                                                m_connector);
            m_variablesByName[paramName] = varId;
			m_localVariables.push_back(varId);
         }
         else
            checkError(false, 
                       "Unexpected expression type '" << alias->second->name()
                       << "' in: " << getNodeId().toString());
      }
      
      m_children.push_back((new Node(body->libNode(), m_exec, m_id))->getId());

      ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
      ExpressionListenerId listener = m_listeners[childrenWaitingOrFinishedIdx];
      cond->addListener(listener);
      m_conditions[childrenWaitingOrFinishedIdx] = cond;
      m_garbageConditions.insert(childrenWaitingOrFinishedIdx);

      ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
      listener = m_listeners[endIdx];
      endCond->addListener(listener);
      m_conditions[endIdx] = endCond;
      m_garbageConditions.insert(endIdx);    
   }

   void Node::getVarsFromInterface(const PlexilInterfaceId& intf)
   {
      if (!intf.isValid())
         return;
      
      debugMsg("Node:getVarsFromInterface",
               "Getting interface vars for node '" << m_nodeId.toString() << "'");
      checkError(m_parent.isId(), "Bizarre.  An interface on a parentless node.");
      //CHECK FOR DUPLICATE NAMES
      for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
		   it != intf->in().end();
		   ++it) 
      {
		PlexilVarRef* varRef = *it;
		VariableId expr = m_parent->findVariable(varRef);
		// FIXME: push this check up into XML parser
		checkError(expr.isId(),
				   "No variable named '" << varRef->name() <<
				   "' in parent of node '" << m_nodeId.toString() << "'");
		// FIXME: push this check up into XML parser
		checkError(Id<VariableImpl>::convertable(expr)
				   || Id<AliasVariable>::convertable(expr),
				   "Expression named '" << varRef->name() <<
				   "' in parent of node '" << m_nodeId.toString() <<
				   "' is not a variable.");


		// Generate a constant alias for this variable
		VariableId alias =
		  (new AliasVariable(varRef->name(),
							 m_connector,
							 (Id<Variable>) expr,
							 true))->getId();

		// add alias to this node
		debugMsg("Node:getVarsFromInterface", 
				 " for node " << m_nodeId.c_str()
				 << ": Adding In variable "
				 << alias->toString()
				 << " as '" << varRef->name()
				 << "'"); 
		m_variablesByName[LabelStr(varRef->name())] = alias;
      }
      
      for(std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
          it != intf->inOut().end(); ++it) 
      {
         VariableId expr = m_parent->findVariable(*it);
		 // FIXME: push this check up into XML parser
         checkError(expr.isId(),
                    "No variable named '" << (*it)->name() <<
                    "' in parent of node '" << m_nodeId.toString() << "'");
         
         // add variable to this node
		 debugMsg("Node:getVarsFromInterface", 
				  " for node '" << m_nodeId.c_str()
				  << "': Adding InOut variable "
				  << expr->toString()
				  << " as '" << (*it)->name()
				  << "'"); 
         m_variablesByName[LabelStr((*it)->name())] = expr;
      }
   }

  void Node::createDeclaredVars(const std::vector<PlexilVarId>& vars) {
    for(std::vector<PlexilVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
       // get the variable name

	  const std::string& name = (*it)->name();
	  LabelStr nameLabel(name);

       // if it's an array, make me an array
       
       if (Id<PlexilArrayVar>::convertable((*it)->getId()))
       {
          PlexilValue* value = (*it)->value();
          VariableId varId =
			(VariableId)
			ExpressionFactory::createInstance(value->name(), 
											  value->getId(),
											  m_connector);
          // FIXME: CHECK FOR DUPLICATE NAMES

          m_variablesByName[nameLabel] = varId;
		  ((VariableImpl*) varId)->setName(name);
          m_localVariables.push_back(varId);
          debugMsg("Node:createDeclaredVars",
                   " for node '" << m_nodeId.toString()
				   << "': created array variable "
                   << varId->toString()  << " as '"
				   << name << "'");
       }
       // otherwise create a non-array variable

       else
       {
          PlexilValue* value = (*it)->value();
          VariableId varId =
			(VariableId)
			ExpressionFactory::createInstance(value->name(), 
											  value->getId(),
											  m_connector);
          // FIXME: CHECK FOR DUPLICATE NAMES

          m_variablesByName[nameLabel] = varId;
		  ((VariableImpl*) varId)->setName(name);
          m_localVariables.push_back(varId);
          debugMsg("Node:createDeclaredVars",
                   " for node '" << m_nodeId.toString()
				   << "': created variable " 
				   << varId->toString() << " as '"
				   << name << "'");
       }
    }
  }

  /**
   * @brief Notifies the node that one of its conditions has changed.
   */
  void Node::conditionChanged()
  {
    if (m_checkConditionsPending)
      return;
    m_exec->notifyNodeConditionChanged(m_id);
    m_checkConditionsPending = true;
  }

  /**
   * @brief Evaluates the conditions to see if the node is eligible to transition.
   */
  void Node::checkConditions() {
    m_checkConditionsPending = false;
    checkError(m_stateVariable->getValue() == StateVariable::nodeStateName(m_state).getKey(),
	       "Node state not synchronized for node " << m_nodeId.toString()
	       << "; node state = " << m_state
	       << ", node state name = \"" << Expression::valueToString(m_stateVariable->getValue()) << "\"");

    if(m_transitioning)
      return;

    debugMsg("Node:checkConditions",
	     "Checking condition change for node " << m_nodeId.toString());
    NodeState toState(getDestState());
    debugMsg("Node:checkConditions",
	     "Can (possibly) transition to " << StateVariable::nodeStateName(toState).toString());
    if (toState != m_lastQuery) {
      if (toState != NO_NODE_STATE ||
	  (toState == NO_NODE_STATE &&
	   m_lastQuery != NO_NODE_STATE))
	m_exec->handleConditionsChanged(m_id);
      m_lastQuery = toState;
    }
  }

  void Node::transition(const double time) {
    checkError(m_stateVariable->getValue() == StateVariable::nodeStateName(m_state).getKey(),
	       "Node state not synchronized for node " << m_nodeId.toString()
	       << "; node state = " << m_state
	       << ", node state name = \"" << Expression::valueToString(m_stateVariable->getValue()) << "\"");
    checkError(!m_transitioning,
	       "Node " << m_nodeId.toString() << " is already transitioning.");
    m_transitioning = true;
    NodeState prevState(getState());
    m_stateManager->transition(m_id);
    NodeState newState(getState());
    debugMsg("Node:transition", "Transitioning '" << m_nodeId.toString() <<
	     "' from " << StateVariable::nodeStateName(prevState).toString() <<
	     " to " << StateVariable::nodeStateName(newState).toString());
    condDebugMsg((newState == FINISHED_STATE),
                 "Node:outcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    condDebugMsg((newState == FINISHED_STATE && getOutcome() == OutcomeVariable::FAILURE()),
                 "Node:failure",
                 "Failure type of '" << m_nodeId.toString() <<
                 "' is " << getFailureType().toString());
    condDebugMsg((newState == ITERATION_ENDED_STATE),
                 "Node:iterationOutcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    debugMsg("Node:times",
	     "Setting end time " << LabelStr(END_TIMEPOINT_NAMES()[prevState]).toString()
			 << " = " << std::setprecision(15) << time);
    debugMsg("Node:times",
	     "Setting start time " << LabelStr(START_TIMEPOINT_NAMES()[newState]).toString()
			 << " = " << std::setprecision(15) << time);
    m_endTimepoints[prevState]->setValue(time);
    m_startTimepoints[newState]->setValue(time);
    m_transitioning = false;
    conditionChanged(); // was checkConditions();
  }

  const VariableId& Node::getInternalVariable(const LabelStr& name) const{
    checkError(m_variablesByName.find(name) != m_variablesByName.end(),
	       "No variable named " << name.toString() << " in " << m_nodeId.toString());
    return m_variablesByName.find(name)->second;
  }

  const LabelStr& Node::getStateName() const {
    return StateVariable::ALL_STATES()[m_state];
  }

  NodeState Node::getState() const {
    return m_state;
  }

  void Node::setState(NodeState newValue) {
    checkError(newValue < NO_NODE_STATE,
	       "Attempted to set an invalid NodeState value");
    m_state = newValue;
    ((StateVariable*) m_stateVariable)->setNodeState(newValue);
  }

  double Node::getCurrentStateStartTime() const
  {
    return m_startTimepoints[m_state]->getValue();
  }

  double Node::getCurrentStateEndTime() const
  {
    return m_endTimepoints[m_state]->getValue();
  }

  const LabelStr Node::getOutcome() {
    return m_outcomeVariable->getValue();
  }

  const LabelStr Node::getFailureType() {
    return m_failureTypeVariable->getValue();
  }

  const LabelStr Node::getCommandHandle() {
	if (m_commandHandleVariable.isId())
	  return m_commandHandleVariable->getValue();
	else 
	  return Expression::UNKNOWN();
  }

  class NodeIdEq {
  public:
    NodeIdEq(const double name) : m_name(name) {}
    bool operator()(const NodeId& node) {return node->getNodeId() == m_name;}
  private:
    double m_name;
  };

  // Searches ancestors when required
  const VariableId& Node::findVariable(const LabelStr& name, bool recursive)
  {
    debugMsg("Node:findVariable",
			 " for node '" << m_nodeId.toString()
			 << "', searching by name for \"" << name.toString() << "\"");
    VariableMap::const_iterator it = m_variablesByName.find(name.getKey());
	if (it != m_variablesByName.end()) {
	  debugMsg("Node:findVariable",
			   " Returning " << it->second->toString());
	  return it ->second;
	}

	// Not found locally - try ancestors if possible
	// Stop at library call nodes, as interfaces there are explicit
	if (m_parent.isId()
		&& m_parent->m_nodeType != LIBRARYNODECALL()) {
	  const VariableId& result = m_parent->findVariable(name, true);
	  if (result.isId()) {
		// Found it - cache for later reuse
		m_variablesByName[name.getKey()] = result;
		return result;
	  }
	  // Not found 
	  else if (recursive)
		return VariableId::noId(); // so that error happens at approriate level
	  // else fall through to failure
	}

	// FIXME: push this check up into XML parser
	checkError(ALWAYS_FAIL,
			   "No variable named \"" << name.toString() << "\" accessible from node " <<
			   m_nodeId.toString());
	return VariableId::noId();
  }
   
   const VariableId& Node::findVariable(const PlexilVarRef* ref)
   {
	 debugMsg("Node:findVariable",
			  " for node '" << m_nodeId.toString()
			  << "', searching for variable '" << ref->name() << "'");
      
	 if (Id<PlexilInternalVar>::convertable(ref->getId())) {
	   PlexilInternalVar* var = (PlexilInternalVar*) ref;
	   PlexilNodeRef* nodeRef = var->ref();
	   NodeId node = NodeId::noId();
         
	   switch(nodeRef->dir()) {
	   case PlexilNodeRef::SELF:
		 node = m_id;
		 break;

	   case PlexilNodeRef::PARENT:
		 // FIXME: push this check up into XML parser
		 checkError(m_parent.isValid(),
					"Parent node reference in root node " << 
					m_nodeId.toString());
		 node = m_parent;
		 break;

	   case PlexilNodeRef::CHILD:
		 {
		   // FIXME: push this check up into XML parser
		   checkError(m_nodeType == Node::LIST(),
					  "Child internal variable reference in node " << 
					  m_nodeId.toString() <<
					  " which isn't a list node.");
		   std::vector<NodeId>::const_iterator it =
			 std::find_if(m_children.begin(), m_children.end(),
						  NodeIdEq(LabelStr(nodeRef->name())));
		   // FIXME: push this check up into XML parser
		   checkError(it != m_children.end(),
					  "No child named '" << nodeRef->name() << 
					  "' in " << m_nodeId.toString());
		   node = *it;
		   break;
		 }

	   case PlexilNodeRef::SIBLING: 
		 {
		   // FIXME: push this check up into XML parser
		   checkError(m_parent.isValid(),
					  "Sibling node reference in root node " << 
					  m_nodeId.toString());
		   std::vector<NodeId>::const_iterator it =
			 std::find_if(m_parent->m_children.begin(), 
						  m_parent->m_children.end(),
						  NodeIdEq(LabelStr(nodeRef->name())));
		   // FIXME: push this check up into XML parser
		   checkError(it != m_parent->m_children.end(),
					  "No sibling named '" << nodeRef->name() << 
					  "' of " << m_nodeId.toString());
		   node = *it;
		   break;
		 }

	   case PlexilNodeRef::NO_DIR:
	   default:
		 // FIXME: catch this error in XML parsing
		 checkError(ALWAYS_FAIL,
					"Invalid direction in node reference from " <<
					m_nodeId.toString());
		 return VariableId::noId();
	   }

	   std::string name;
	   if(Id<PlexilTimepointVar>::convertable(var->getId())) {
		 PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
		 name = tp->state() + "." + tp->timepoint();
	   }
	   else
		 name = var->name();
	   debugMsg("Node:findVariable", 
				" Found internal variable \"" << name << "\"");
	   return node->getInternalVariable(LabelStr(name));
	 }
	 else {
	   return findVariable(LabelStr(ref->name()));
	 }
   }

  //cheesy hack?
  CommandId& Node::getCommand() {
    if(m_state == EXECUTING_STATE)
      m_command->activate();
    if(m_command.isValid()) {
      m_command->fixValues();
      m_command->fixResourceValues();
    }
    return m_command;
  }

  UpdateId& Node::getUpdate() {
    if(m_state == EXECUTING_STATE)
      m_update->activate();
    if(m_update.isValid())
      m_update->fixValues();
    return m_update;
  }

  const VariableId& Node::getAssignmentVariable() const {return m_assignment->getDest();}

  AssignmentId& Node::getAssignment() {
    check_error(getType() == ASSIGNMENT());
    if(m_state == EXECUTING_STATE)
      m_assignment->activate();
    m_assignment->fixValue();
    return m_assignment;
  }

  NodeState Node::getDestState() 
  {
    debugMsg("Node:getDestState",
	     "Getting destination state for " << m_nodeId.toString() << " from state " <<
	     getStateName().toString());
    return m_stateManager->getDestState(m_id);
  }

  void Node::activatePair(unsigned int idx) {
    checkError(m_listeners[idx].isId() && m_conditions[idx].isId(),
			   "No condition/listener pair exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:activatePair",
			 "Activating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    m_listeners[idx]->activate();
    m_conditions[idx]->activate();
  }

  void Node::deactivatePair(unsigned int idx) {
    checkError(m_listeners[idx].isId() && m_conditions[idx].isId(),
			   "No condition/listener pair exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:deactivatePair",
			 "Deactivating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    m_conditions[idx]->deactivate();
    if (m_listeners[idx]->isActive())
      m_listeners[idx]->deactivate();
  }

  bool Node::pairActive(unsigned int idx) {
    checkError(idx < conditionIndexMax,
	       "Invalid condition index " << idx);
    bool listenActive = m_listeners[idx]->isActive();
    condDebugMsg(!listenActive, 
		 "Node:pairActive",
		 "Listener for " << getConditionName(idx).toString() << " in " << m_nodeId.toString() <<
		 " is inactive.");
    bool condActive = m_conditions[idx]->isActive();
    condDebugMsg(!condActive, 
		 "Node:pairActive",
		 "Condition " << getConditionName(idx).toString() << " in " << m_nodeId.toString() <<
		 " is inactive.");
    return listenActive && condActive;
  }

  void Node::execute() {
    debugMsg("Node:execute", "Executing node " << m_nodeId.toString());
    // activate local variables
    for (std::vector<VariableId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         vit++)
      {
        (*vit)->activate();
      }
    m_exec->handleNeedsExecution(m_id);
  }

  void Node::reset() {
    debugMsg("Node:reset", "Re-setting node " << m_nodeId.toString());

    //reset outcome and failure type
	m_outcomeVariable->reset();
	m_failureTypeVariable->reset();
	if (m_commandHandleVariable.isId())
	  m_commandHandleVariable->reset();

    //reset timepoints
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      m_startTimepoints[s]->reset();
      m_endTimepoints[s]->reset();
    }

    for (std::vector<VariableId>::const_iterator it = m_localVariables.begin();
		 it != m_localVariables.end();
		 ++it)
      (*it)->reset();
    
    if (getType() == COMMAND()
		|| getType() == UPDATE()
		|| getType() == REQUEST())
      m_ack->reset();
  }

  void Node::abort() {
    debugMsg("Node:abort", "Aborting node " << m_nodeId.toString());
    if(getType() == Node::COMMAND() && m_command.isValid())
      m_exec->getExternalInterface()->invokeAbort(m_command->getName(),
                                                  m_command->getArgValues(),
                                                  m_conditions[abortCompleteIdx],
                                                  m_command->m_ack);
    else if(getType() == Node::ASSIGNMENT() && m_assignment.isValid())
      m_assignment->getDest()->setValue(Expression::UNKNOWN());
    else {
      condDebugMsg(getType() == Node::COMMAND() && m_command.isInvalid(),
		   "Warning", "Invalid command id in " << m_nodeId.toString());
      condDebugMsg(getType() == Node::ASSIGNMENT() && m_assignment.isInvalid(),
		   "Warning", "Invalid assignment id in " << m_nodeId.toString());
      debugMsg("Warning", "No abort for node type " << getType().toString() << " yet.");
      //checkError(ALWAYS_FAIL, "No abort currently for node type " << getType().toString());
    }
  }

  /* old version
  void Node::lockConditions() {
    for(ExpressionMap::iterator it = m_conditionsByName.begin();
	it != m_conditionsByName.end(); ++it) {
      ExpressionId expr = it->second;
      check_error(expr.isValid());
      if(pairActive(it->first) && !expr->isLocked()) {
	debugMsg("Node:lockConditions",
		 "In " << m_nodeId.toString() << ", locking " <<
		 LabelStr(it->first).toString() << " " << expr->toString());
	expr->lock();
      }
    }
  }
  */

  // Optimized version
  // N.B. we omit the validity check on the condition expression
  // because this is a critical path method in the inner loop of the Exec.
  void Node::lockConditions() {
    for (unsigned int i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = m_conditions[i];
      checkError(m_listeners[i].isId(), // isValid() ??
		 "Node::lockConditions: no listener named " << getConditionName(i).toString());
      if (m_listeners[i]->isActive()
	  && expr->isActive()
	  && !expr->isLocked()) {
	debugMsg("Node:lockConditions",
		 "In " << m_nodeId.toString() << ", locking " <<
		 getConditionName(i).toString() << " " << expr->toString());
	expr->lock();
      }
    }
  }

  // As above, skip the Id validity check because this is a critical path function.
  void Node::unlockConditions() {
    for (unsigned int i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = m_conditions[i];
      // check_error(expr.isValid());
      if(expr->isLocked()) {
	debugMsg("Node:unlockConditions",
		 "In " << m_nodeId.toString() << ", unlocking " <<
		 getConditionName(i).toString() << " " << expr->toString());
	expr->unlock();
      }
    }
  }

  void Node::deactivateExecutable() {
    if(getType() == Node::COMMAND() && m_command.isValid())
      m_command->deactivate();
    else if(getType() == Node::ASSIGNMENT() && m_assignment.isValid())
      m_assignment->deactivate();
    else if(getType() == Node::UPDATE() && m_update.isValid())
      m_update->deactivate();
    // deactivate local variables
    for (std::vector<VariableId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         vit++)
	  (*vit)->deactivate();
  }

  std::string Node::toString(const unsigned int indent)
  {
    std::ostringstream retval;
	print(retval, indent);
    return retval.str();
  }

  std::ostream& operator<<(std::ostream& stream, const Node& node)
  {
	node.print(stream, 0);
	return stream;
  }

  void Node::print(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');

    stream << indentStr << m_nodeId.toString() << "{\n";
    stream << indentStr << " State: " << m_stateVariable->toString() <<
      " (" << m_startTimepoints[m_state]->getValue() << ")\n";
    if (m_state == FINISHED_STATE) {
      stream << indentStr << " Outcome: " << m_outcomeVariable->toString() << '\n';
      if (m_failureTypeVariable->getValue() != Expression::UNKNOWN())
		stream << indentStr << " Failure type: " <<
		  m_failureTypeVariable->toString() << '\n';
	  // Print variables, starting with command handle
      if (m_nodeType == COMMAND() && m_commandHandleVariable->getValue() != Expression::UNKNOWN())
		stream << indentStr << " Command handle: " <<
		  m_commandHandleVariable->toString() << '\n';
	  printVariables(stream, indent);
    }
    else if (m_state != INACTIVE_STATE) {
	  // Print conditions
      for (unsigned int i = 0; i < conditionIndexMax; ++i) {
		stream << indentStr << " " << getConditionName(i).toString() << ": " <<
		  m_conditions[i]->toString() << '\n';
      }
	  // Print variables, starting with command handle (if appropriate)
      if (m_nodeType == COMMAND()) {
		stream << indentStr << " Command handle: " <<
		  m_commandHandleVariable->toString() << '\n';
	  }
	  printVariables(stream, indent);
    }
	// print children
    for(std::vector<NodeId>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
      stream << (*it)->toString(indent + 2);
    }
    stream << indentStr << "}" << std::endl;
  }

  // Print variables
  void Node::printVariables(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
	ensureSortedVariableNames(); // for effect
	for (std::vector<double>::const_iterator it = m_sortedVariableNames->begin();
		 it != m_sortedVariableNames->end();
		 it++) {
	  stream << indentStr << " " << LabelStr(*it).toString() << ": " <<
		getInternalVariable(LabelStr(*it))->toString() << '\n';
	}
  }

  // Helper used below
  bool labelStrLessThan(double a, double b)
  {
	return LabelStr(a).toString() < LabelStr(b).toString();
  }

  void Node::ensureSortedVariableNames() const
  {
	checkError(m_sortedVariableNames != NULL,
			   "Internal error: m_sortedVariableNames is null!");
	if (m_sortedVariableNames->empty()) {
	  // Collect the variable names
	  for (VariableMap::const_iterator it = m_variablesByName.begin();
		   it != m_variablesByName.end();
		   it++) {
		double nameKey = it->first;
		if (nameKey == STATE().getKey()
			|| nameKey == OUTCOME().getKey()
			|| nameKey == FAILURE_TYPE().getKey()
			|| nameKey == COMMAND_HANDLE().getKey()
			|| LabelStr(nameKey).countElements(".") > 1)
		  continue;
		m_sortedVariableNames->push_back(it->first);
	  }
	  // Sort the names
	  std::sort(m_sortedVariableNames->begin(),
				m_sortedVariableNames->end(),
				labelStrLessThan);
	}
  }

  // Static "constants"
  const std::vector<double>& Node::START_TIMEPOINT_NAMES() {
    static std::vector<double>* startNames = NULL;
    if (startNames == NULL) {
      startNames = new std::vector<double>();
      startNames->reserve(NODE_STATE_MAX);
      for (std::vector<LabelStr>::const_iterator it = StateVariable::ALL_STATES().begin();
	   it != StateVariable::ALL_STATES().end(); 
	   ++it) {
	const std::string& state = it->toString();
	LabelStr startName(state + ".START");
	startNames->push_back(startName.getKey());
      }
    }
    return *startNames;
  }

  const std::vector<double>& Node::END_TIMEPOINT_NAMES() {
    static std::vector<double>* endNames = NULL;
    if (endNames == NULL) {
      endNames = new std::vector<double>();
      endNames->reserve(NODE_STATE_MAX);
      for (std::vector<LabelStr>::const_iterator it = StateVariable::ALL_STATES().begin();
	   it != StateVariable::ALL_STATES().end(); 
	   ++it) {
	const std::string& state = it->toString();
	LabelStr endName(state + ".END");
	endNames->push_back(endName.getKey());
      }
    }
    return *endNames;
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
		 ++it)
      delete (Expression*) (*it);
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

  Assignment::Assignment(const VariableId lhs, 
						 const ExpressionId rhs,
						 const VariableId ack, 
						 const LabelStr& lhsName, 
                         const bool deleteLhs, 
						 const bool deleteRhs)
    : m_id(this), m_lhs(lhs), m_rhs(rhs), m_ack(ack), 
      m_value(Expression::UNKNOWN()),
      m_destName(lhsName),
      m_deleteLhs(deleteLhs), m_deleteRhs(deleteRhs)
  {
  }

  Assignment::~Assignment() {
    if(m_deleteLhs)
      delete (Expression*) m_lhs;
    if(m_deleteRhs)
      delete (Expression*) m_rhs;
    m_id.remove();
  }

  void Assignment::fixValue() {
    m_value = m_rhs->getValue();
  }

  void Assignment::activate() {
    m_rhs->activate();
    m_lhs->activate();
  }

  void Assignment::deactivate() {
    m_rhs->deactivate();
    m_lhs->deactivate();
  }

  const std::string& Assignment::getDestName() {
    return m_destName.toString();
  }

  Update::Update(const NodeId& node, 
				 const ExpressionMap& pairs,
				 const VariableId ack,
				 const std::list<ExpressionId>& garbage)
    : m_id(this), m_source(node), m_pairs(pairs), m_ack(ack), m_garbage(garbage) {}

  Update::~Update() {
    for(std::list<ExpressionId>::const_iterator it = m_garbage.begin(); it != m_garbage.end();
	++it)
      delete (Expression*) (*it);
    m_id.remove();
  }

  void Update::fixValues() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end();
	++it) {
      check_error(it->second.isValid());
      std::map<double, double>::iterator valuePairIt =
	m_valuePairs.find(it->first);
      if (valuePairIt == m_valuePairs.end())
	{
	  // new pair, safe to insert
	  m_valuePairs.insert(std::make_pair(it->first, it->second->getValue()));
	}
      else
	{
	  // recycle old pair
	  valuePairIt->second = it->second->getValue();
	}
      debugMsg("Update:fixValues",
	       " fixing pair '" << LabelStr(it->first).toString() << "', "
	       << it->second->getValue());
    }
  }

  void Update::activate() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->activate();
    }
  }

  void Update::deactivate() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->deactivate();
    }
  }
}
