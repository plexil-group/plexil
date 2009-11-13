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

#include "Node.hh"
#include "Expression.hh"
#include "CoreExpressions.hh"
#include "Expressions.hh"
#include "XMLUtils.hh"
#include "PlexilExec.hh"
#include "ExternalInterface.hh"
#include "Debug.hh"
#include "NodeStateManager.hh"
#include "CommandHandle.hh"

#include <vector>
#include <sstream>

namespace PLEXIL {
  const std::set<double>& Node::ALL_CONDITIONS() {
    static bool init = true;
    static std::set<double> sl_allConds;
    if(init) {
      sl_allConds.insert(SKIP_CONDITION());
      sl_allConds.insert(START_CONDITION());
      sl_allConds.insert(END_CONDITION());
      sl_allConds.insert(INVARIANT_CONDITION());
      sl_allConds.insert(PRE_CONDITION());
      sl_allConds.insert(POST_CONDITION());
      sl_allConds.insert(REPEAT_CONDITION());
      sl_allConds.insert(ANCESTOR_INVARIANT_CONDITION());
      sl_allConds.insert(ANCESTOR_END_CONDITION());
      sl_allConds.insert(PARENT_EXECUTING_CONDITION());
      sl_allConds.insert(CHILDREN_WAITING_OR_FINISHED());
      sl_allConds.insert(ABORT_COMPLETE());
      sl_allConds.insert(PARENT_WAITING_CONDITION());
      sl_allConds.insert(PARENT_FINISHED_CONDITION());
      sl_allConds.insert(COMMAND_HANDLE_RECEIVED_CONDITION());
      init = false;
    }
    return sl_allConds;
  }

  class ConditionChangeListener : public ExpressionListener {
  public:
    ConditionChangeListener(NodeId node, const LabelStr& cond)
      : ExpressionListener(), m_node(node), m_cond(cond) {}
    void notifyValueChanged(const ExpressionId& expression) {
      debugMsg("Node:conditionChange",
	       m_cond.toString() << " may have changed value in " <<
	       m_node->getNodeId().toString());

      m_node->checkConditions();
    }
  protected:
  private:
    NodeId m_node;
    LabelStr m_cond;
  };

  class RealNodeConnector : public NodeConnector {
  public:
    RealNodeConnector(const NodeId& node) 
      : NodeConnector(), m_node(node) 
    {
    }

    const ExpressionId& findVariable(const PlexilVarRef* ref) const
    {
      return m_node->findVariable(ref);
    }

    const ExpressionId& findVariable(const LabelStr& name) const
    {
      return m_node->findVariable(name);
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
    switch(nodeType)
      {
      case NodeType_NodeList:
        return LIST();
        break;
      case NodeType_Command:
        return COMMAND();
        break;
      case NodeType_Assignment:
        return ASSIGNMENT();
        break;
      case NodeType_FunctionCall:
        return FUNCTION();
        break;
      case NodeType_Update:
        return UPDATE();
        break;
      case NodeType_Request:
        return REQUEST();
        break;
      case NodeType_Empty:
        return EMPTY();
        break;
      case NodeType_LibraryNodeCall:
        return LIBRARYNODECALL();
        break;

        // fall thru case
      default:
        checkError(ALWAYS_FAIL,
                   "Invalid node type " << nodeType);
        break;
      }
    return EMPTY_LABEL();
  }


  unsigned int Node::anonynode = 0;

  Node::Node(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent)
    : m_id(this), m_exec(exec), m_parent(parent),
      m_connector((new RealNodeConnector(m_id))->getId()), m_node(node), sl_called(false),
      m_cleanedConditions(false), m_cleanedVars(false), m_transitioning(false),
      m_lastQuery(StateVariable::UNKNOWN()), m_priority(WORST_PRIORITY)
  {
     m_nodeId = LabelStr(node->nodeId());

     m_priority = node->priority();

     m_nodeType = nodeTypeToLabelStr(node->nodeType());

     debugMsg("Node:node", "Creating node '" << m_nodeId.toString() << "'");
     commonInit();

        //instantiate declared variables

     createDeclaredVars(node->declarations());

        //get interface variables

     getVarsFromInterface(node->interface());

        // inherit all variables from parten which
        // do not already appear in this node 

     getVarsFromParent();

        //instantiate child nodes, if any (have to create assignments and commands after
        //everything else
        //because they could refer to internal variables of other nodes)

     if(m_nodeType == LIST()) 
     {
        debugMsg("Node:node", "Creating child nodes.");
	checkError(Id<PlexilListBody>::convertable(node->body()),
		   "Node " << m_nodeId.toString() << " is a list node but doesn't have a " <<
		   "list body.");
        createChildNodes((PlexilListBody*) node->body());
     }
        // create library call node

     if(m_nodeType == LIBRARYNODECALL()) 
     {
        debugMsg("Node:node", "Creating library node call.");
	checkError(Id<PlexilLibNodeCallBody>::convertable(node->body()),
		   "Node " << m_nodeId.toString() << " is a list node but doesn't have a " <<
		   "list body.");
        createLibraryNode(node);
     }
  }

  Node::Node(const LabelStr& type, const LabelStr& name, const LabelStr& state,
	     const bool skip, const bool start, const bool pre, const bool invariant, const bool post,
	     const bool end, const bool repeat, const bool ancestorInvariant,
	     const bool ancestorEnd, const bool parentExecuting, const bool childrenFinished,
	     const bool commandAbort, const bool parentWaiting, 
	     const bool parentFinished, const bool cmdHdlRcvdCondition, const ExecConnectorId& exec)
    : m_id(this), m_exec(exec), m_parent(NodeId::noId()), m_node(PlexilNodeId::noId()),
      sl_called(false), m_cleanedConditions(false), m_cleanedVars(false),
      m_transitioning(false), m_lastQuery(StateVariable::UNKNOWN()) {
    m_nodeType = type;
    m_nodeId = name;
    commonInit();
    double conds[15] = {SKIP_CONDITION(), START_CONDITION(), PRE_CONDITION(), INVARIANT_CONDITION(),
			POST_CONDITION(), END_CONDITION(), REPEAT_CONDITION(),
			ANCESTOR_INVARIANT_CONDITION(), ANCESTOR_END_CONDITION(),
			PARENT_EXECUTING_CONDITION(), CHILDREN_WAITING_OR_FINISHED(),
			ABORT_COMPLETE(), PARENT_WAITING_CONDITION(),
			PARENT_FINISHED_CONDITION(), COMMAND_HANDLE_RECEIVED_CONDITION()};
    bool values[15] = {skip, start, pre, invariant, post, end, repeat, ancestorInvariant,
		       ancestorEnd, parentExecuting, childrenFinished, commandAbort,
		       parentWaiting, parentFinished, cmdHdlRcvdCondition};
    m_variablesByName[STATE()]->setValue(state);
    for(int i = 0; i < 15; i++) {
      debugMsg("Node:node",
	       "Creating internal variable " << LabelStr(conds[i]).toString() <<
	       " with value " << values[i] << " for node " << m_nodeId.toString());
      m_conditionsByName[conds[i]]->removeListener(m_listenersByName[conds[i]]);
      delete (Expression*) m_conditionsByName[conds[i]];
      ExpressionId expr = (new BooleanVariable((double) values[i]))->getId();
      m_conditionsByName[conds[i]] = expr;
      expr->addListener(m_listenersByName[conds[i]]);
      m_garbage.insert(conds[i]);
    }
    if (m_nodeType == COMMAND())
      m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();
    else
      m_ack = (new BooleanVariable(BooleanVariable::UNKNOWN()))->getId();
  }

  Node::~Node() {
    delete (RealNodeConnector*) m_connector;
    cleanUpConditions();
    cleanUpVars();

    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }

    m_id.remove();
  }

  void Node::cleanUpConditions() {
    if(m_cleanedConditions)
      return;

    for(std::map<double, ExpressionListenerId>::iterator it = m_listenersByName.begin();
	it != m_listenersByName.end(); ++it) {
      if(m_variablesByName.find(it->first) != m_variablesByName.end()) {
	debugMsg("Node:cleanUpConds",
		 "<" << m_nodeId.toString() << "> Removing variable listener for " <<
		 LabelStr(it->first).toString());
	m_variablesByName[it->first]->removeListener(it->second);
      }
      else if(m_conditionsByName.find(it->first) != m_conditionsByName.end()) {
	debugMsg("Node:cleanUpConds",
		 "<" << m_nodeId.toString() << "> Removing condition listener for " <<
		 LabelStr(it->first).toString());
	m_conditionsByName[it->first]->removeListener(it->second);
      }
      else {
	checkError(ALWAYS_FAIL,
		   "Have a listener but no expression for " <<
		   LabelStr(it->first).toString());
      }
      delete (ExpressionListener*) it->second;
    }
    m_listenersByName.clear();

    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();

    for(std::set<double>::iterator it = m_garbage.begin(); it != m_garbage.end(); ++it) {
      if(m_conditionsByName.find(*it) != m_conditionsByName.end()) {
	debugMsg("Node:cleanUpConds",
		 "<" << m_nodeId.toString() << "> Removing " << LabelStr(*it).toString());
	delete (Expression*) m_conditionsByName.find(*it)->second;
	m_conditionsByName.erase(*it);
      }
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
    checkError(m_cleanedConditions,
	       "Have to clean up variables before conditions can be cleaned.");
    if(m_cleanedVars)
      return;

    if (m_allCommandHandleValues.isValid())
      delete (Expression*) m_allCommandHandleValues;

    if(m_conjunctCondition.isValid())
      delete (Expression*) m_conjunctCondition;

    if(m_extraEndCond.isValid())
      delete (Expression*) m_extraEndCond;

    if(m_interruptEndCond.isValid())
      delete (Expression*) m_interruptEndCond;

    for(std::set<double>::iterator it = m_garbage.begin(); it != m_garbage.end(); ++it) {
      if(m_variablesByName.find(*it) != m_variablesByName.end()) {
	debugMsg("Node:cleanUpVars",
		 "<" << m_nodeId.toString() << "> Removing " << LabelStr(*it).toString());
	delete (Expression*) m_variablesByName.find(*it)->second;
	m_variablesByName.erase(*it);
      }
    }

    if(m_ack.isValid())
      delete (Expression*) m_ack;

    m_cleanedVars = true;
  }

  void Node::commonInit() {
    debugMsg("Node:node", "Instantiating internal variables...");
    //instantiate state/outcome/failure variables
    m_variablesByName[STATE()] = (new StateVariable())->getId();
    m_variablesByName[STATE()]->activate();

    m_variablesByName[OUTCOME()] = (new OutcomeVariable())->getId();
    m_variablesByName[OUTCOME()]->activate();

    m_variablesByName[FAILURE_TYPE()] = (new FailureVariable())->getId();
    m_variablesByName[FAILURE_TYPE()]->activate();

    m_variablesByName[COMMAND_HANDLE()] = (new CommandHandleVariable())->getId();
    m_variablesByName[COMMAND_HANDLE()]->activate();

    m_garbage.insert(STATE());
    m_garbage.insert(OUTCOME());
    m_garbage.insert(FAILURE_TYPE());
    m_garbage.insert(COMMAND_HANDLE());

    debugMsg("Node:node", "Instantiating timepoint variables.");
    //instantiate timepoint variables
    std::vector<std::string> suffix;
    suffix.push_back("START");
    suffix.push_back("END");

    for(std::set<double>::const_iterator it = StateVariable::ALL_STATES().begin();
	it != StateVariable::ALL_STATES().end(); ++it) {
      //       if(*it == StateVariable::INACTIVE())
      //  continue;
      for(unsigned int i = 0; i < suffix.size(); i++) {
	std::stringstream str;
	str << (LabelStr(*it)).toString() << "." << suffix[i];
	LabelStr varName(str.str());
	m_variablesByName[varName] = (new RealVariable())->getId();
	m_variablesByName[varName]->activate();
	m_garbage.insert(varName);
      }
    }
    setConditionDefaults();
  }

  void Node::setConditionDefaults() {
    m_conditionsByName[SKIP_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::FALSE(), true))->getId();
    m_listenersByName[SKIP_CONDITION()] =
      (new ConditionChangeListener(m_id, SKIP_CONDITION()))->getId();
    m_conditionsByName[START_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[START_CONDITION()] =
      (new ConditionChangeListener(m_id, START_CONDITION()))->getId();
    m_conditionsByName[END_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[END_CONDITION()] =
      (new ConditionChangeListener(m_id, END_CONDITION()))->getId();
    m_conditionsByName[INVARIANT_CONDITION()] =
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[INVARIANT_CONDITION()] =
      (new ConditionChangeListener(m_id, INVARIANT_CONDITION()))->getId();
    m_conditionsByName[PRE_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[PRE_CONDITION()] =
      (new ConditionChangeListener(m_id, PRE_CONDITION()))->getId();
    m_conditionsByName[POST_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[POST_CONDITION()] =
      (new ConditionChangeListener(m_id, POST_CONDITION()))->getId();
    m_conditionsByName[REPEAT_CONDITION()] =
      (new BooleanVariable(BooleanVariable::FALSE(), true))->getId();
    m_listenersByName[REPEAT_CONDITION()] =
      (new ConditionChangeListener(m_id, REPEAT_CONDITION()))->getId();
    m_conditionsByName[ANCESTOR_INVARIANT_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[ANCESTOR_INVARIANT_CONDITION()] =
      (new ConditionChangeListener(m_id, ANCESTOR_INVARIANT_CONDITION()))->getId();
    m_conditionsByName[ANCESTOR_END_CONDITION()] =
      (new BooleanVariable(BooleanVariable::FALSE(), true))->getId();
    m_listenersByName[ANCESTOR_END_CONDITION()] =
      (new ConditionChangeListener(m_id, ANCESTOR_END_CONDITION()))->getId();
    m_conditionsByName[PARENT_EXECUTING_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[PARENT_EXECUTING_CONDITION()] =
      (new ConditionChangeListener(m_id, PARENT_EXECUTING_CONDITION()))->getId();
    m_conditionsByName[CHILDREN_WAITING_OR_FINISHED()] =
      (new BooleanVariable(BooleanVariable::UNKNOWN(), true))->getId();
    m_listenersByName[CHILDREN_WAITING_OR_FINISHED()] =
      (new ConditionChangeListener(m_id, CHILDREN_WAITING_OR_FINISHED()))->getId();
    m_conditionsByName[ABORT_COMPLETE()] =
      (new BooleanVariable(BooleanVariable::UNKNOWN(), true))->getId();
    m_listenersByName[ABORT_COMPLETE()] =
      (new ConditionChangeListener(m_id, CHILDREN_WAITING_OR_FINISHED()))->getId();
    m_conditionsByName[PARENT_WAITING_CONDITION()] =
      (new BooleanVariable(BooleanVariable::FALSE(), true))->getId();
    m_listenersByName[PARENT_WAITING_CONDITION()] =
      (new ConditionChangeListener(m_id, PARENT_WAITING_CONDITION()))->getId();
    m_conditionsByName[PARENT_FINISHED_CONDITION()] =
      (new BooleanVariable(BooleanVariable::FALSE(), true))->getId();
    m_listenersByName[PARENT_FINISHED_CONDITION()] =
      (new ConditionChangeListener(m_id, PARENT_FINISHED_CONDITION()))->getId();
    m_conditionsByName[COMMAND_HANDLE_RECEIVED_CONDITION()] = 
      (new BooleanVariable(BooleanVariable::TRUE(), true))->getId();
    m_listenersByName[COMMAND_HANDLE_RECEIVED_CONDITION()] =
      (new ConditionChangeListener(m_id, COMMAND_HANDLE_RECEIVED_CONDITION()))->getId();


    m_listenersByName[PARENT_EXECUTING_CONDITION()]->activate();
    m_listenersByName[PARENT_FINISHED_CONDITION()]->activate();
//     m_listenersByName[ANCESTOR_END_CONDITION()]->activate();
//     m_listenersByName[ANCESTOR_INVARIANT_CONDITION()]->activate();
  }

  void Node::postInit() {
    checkError(!sl_called, "Called postInit on node '" << m_nodeId.toString() << "' twice.");
    sl_called = true;

    debugMsg("Node:postInit", "Creating conditions for node '" << m_nodeId.toString() << "'");
    //create conditions and listeners
    createConditions(m_node->conditions());

    //create assignment/command
    if(m_nodeType == COMMAND()) {
      debugMsg("Node:postInit", "Creating command for node '" << m_nodeId.toString() << "'");
      checkError(Id<PlexilCommandBody>::convertable(m_node->body()),
		 "Node is a command node but doesn't have a command body.");
      createCommand((PlexilCommandBody*)m_node->body());
    }
    else if(m_nodeType == ASSIGNMENT()) {
      debugMsg("Node:postInit",
	       "Creating assignment for node '" << m_nodeId.toString() << "'");
      checkError(Id<PlexilAssignmentBody>::convertable(m_node->body()),
		 "Node is an assignment node but doesn't have an assignment body.");
      createAssignment((PlexilAssignmentBody*)m_node->body());
    }
    else if(m_nodeType == UPDATE()) {
      debugMsg("Node:postInit", "Creating update for node '" << m_nodeId.toString() << "'");
      checkError(Id<PlexilUpdateBody>::convertable(m_node->body()),
		 "Node is an update node but doesn't have an update body.");
      createUpdate((PlexilUpdateBody*)m_node->body());
    }
    else if(m_nodeType == FUNCTION()) {
      debugMsg("Node:postInit", "Creating function call for node '" << m_nodeId.toString() << "'");
      checkError(Id<PlexilFunctionCallBody>::convertable(m_node->body()),
		 "Node is a function call node but doesn't have a function call body.");
      createFunctionCall((PlexilFunctionCallBody*)m_node->body());
    }

    //call postInit on all children
    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->postInit();
  }


  void Node::createAssignment(const PlexilAssignmentBody* body) {
    checkError(m_nodeType == ASSIGNMENT(),
	       "Attempted to create an assignment for a(n) " << m_nodeType.toString() <<
	       " node '" << m_nodeId.toString() << "'");

    //we still only support one variable on the LHS
    checkError(body->dest().size() >= 1,
	       "Need at least one destination variable in assignment.");
    const PlexilExprId& destExpr = (body->dest())[0]->getId();
    ExpressionId dest;
    LabelStr destName;
    bool deleteLhs = false;
    if (Id<PlexilVarRef>::convertable(destExpr)) {
      destName = destExpr->name();
      dest = findVariable((Id<PlexilVarRef>) destExpr);
      checkError(dest.isValid(),
                 "Dest variable '" << destName <<
                 "' not found in assignment node '" << m_nodeId.toString() << "'");
    }
    else if (Id<PlexilArrayElement>::convertable(destExpr)) {
      dest = ExpressionFactory::createInstance(destExpr->name(),
                                               destExpr,
                                               m_connector);
      // *** beef this up later ***
      destName = LabelStr("ArrayElement");
      deleteLhs = true;
    }
    else {
      checkError(ALWAYS_FAIL, "Invalid left-hand side to an assignment");
    }

    ExpressionId rhs;
    bool deleteRhs = false;
    if (Id<PlexilVarRef>::convertable(body->RHS())) {
      rhs = findVariable(body->RHS());
    }
    else {
      rhs = ExpressionFactory::createInstance(body->RHS()->name(), body->RHS(),
					      m_connector);
      deleteRhs = true;
    }

    m_assignment =
      (new Assignment(dest, rhs, m_ack, destName, deleteLhs, deleteRhs))->getId();
  }

  void Node::createCommand(const PlexilCommandBody* command) 
  {
     checkError(m_nodeType == COMMAND(),
                "Attempted to create a command for a(n) " << m_nodeType.toString() <<
                " node '" << m_nodeId.toString() << "'");
     checkError(command->state()->nameExpr().isValid(),
                "Attempt to create command with invalid name expression");

    PlexilStateId state = command->state();
    ExpressionId nameExpr = ExpressionFactory::createInstance(
       state->nameExpr()->name(), state->nameExpr(), m_connector);
    LabelStr name(nameExpr->getValue());
    std::list<ExpressionId> args;
    std::list<ExpressionId> garbage;
    for(std::vector<PlexilExprId>::const_iterator it = state->args().begin();
	it != state->args().end(); ++it) 
    {
       ExpressionId argExpr;
       if(Id<PlexilVarRef>::convertable(*it)) 
       {
          argExpr = findVariable(*it);
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
    
    ExpressionId dest;
    if (!command->dest().empty())
      {
        const PlexilExprId& destExpr = command->dest()[0]->getId();
        if (Id<PlexilVarRef>::convertable(destExpr))
          {
            dest = findVariable((Id<PlexilVarRef>) destExpr);
            checkError(dest.isValid(),
                       "Unknown destination variable '" << destExpr->name() <<
                       "' in command '" << name.toString() << "' in node '" <<
                       m_nodeId.toString() << "'");
          }
        else if (Id<PlexilArrayElement>::convertable(destExpr))
          {
            dest = ExpressionFactory::createInstance(destExpr->name(),
                                                     destExpr,
                                                     m_connector);
            garbage.push_back(dest);
          }
        else {
          checkError(ALWAYS_FAIL, "Invalid left-hand side for a command");
        }
      }

    // Resource
    std::vector<std::map<std::string, ExpressionId> > resourceVector;
    const std::vector<PlexilResourceId>& resourceList = command->getResource();
    for(std::vector<PlexilResourceId>::const_iterator resListItr = resourceList.begin();
        resListItr != resourceList.end(); ++resListItr)
      {
        std::map<std::string, ExpressionId> resourceMap;

        const std::map<std::string, PlexilExprId>& resources = (*resListItr)->getResourceMap();
        for(std::map<std::string,PlexilExprId>::const_iterator resItr = resources.begin();
            resItr != resources.end(); ++resItr)
          {
            ExpressionId resExpr;
            if(Id<PlexilVarRef>::convertable(resItr->second))
              {
                resExpr = findVariable(resItr->second);
                checkError(resExpr.isValid(),
                           "Unknown variable '" << resItr->second->name() <<
                           "' in resource list for command '" << nameExpr->getValue() <<
                           "' in node '" << m_nodeId.toString() << "'");
              }
            else 
              {
                resExpr = ExpressionFactory::createInstance(resItr->second->name(), 
                                                            resItr->second, m_connector);
                garbage.push_back(resExpr);
                check_error(resExpr.isValid());
              }
            resourceMap[resItr->first] = resExpr;
          }
        resourceVector.push_back(resourceMap);
      }

    debugMsg("Node:createCommand",
	     "Creating command '" << name.toString() << "' for node '" <<
	     m_nodeId.toString() << "'");
    m_command = (new Command(nameExpr, args, dest, m_ack, garbage, resourceVector))->getId();
    check_error(m_command.isValid());
  }

  void Node::createUpdate(const PlexilUpdateBody* body) {
    checkError(m_nodeType == UPDATE(),
	       "Attempted to create an update for a(n) " << m_nodeType.toString() <<
	       " node '" << m_nodeId.toString() << "'");
    
    PlexilUpdateId update = body->update();
    std::map<double, ExpressionId> updatePairs;
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

  void Node::createFunctionCall(const PlexilFunctionCallBody* funcCall) {
    checkError(m_nodeType == FUNCTION(),
	       "Attempted to create a function call for a(n) " << m_nodeType.toString() <<
	       " node '" << m_nodeId.toString() << "'");
    checkError(funcCall->state()->nameExpr().isValid(),
               "Attempt to create command with invalid name expression");

    PlexilStateId state = funcCall->state();
    ExpressionId nameExpr = 
      ExpressionFactory::createInstance(state->nameExpr()->name(), state->nameExpr(), m_connector);
    LabelStr name(nameExpr->getValue());

    std::list<ExpressionId> args;
    std::list<ExpressionId> garbage;
    for(std::vector<PlexilExprId>::const_iterator it = state->args().begin();
	it != state->args().end(); ++it) {
      ExpressionId argExpr;
      if(Id<PlexilVarRef>::convertable(*it)) {
	argExpr = findVariable(*it);
	checkError(argExpr.isValid(),
		   "Unknown variable '" << (*it)->name() <<
		   "' in argument list for function call '" << name.toString() <<
		   "' in node '" << m_nodeId.toString() << "'");
      }
      else {
	argExpr = ExpressionFactory::createInstance((*it)->name(), *it, m_connector);
	garbage.push_back(argExpr);
	check_error(argExpr.isValid());
      }
      args.push_back(argExpr);
    }

    ExpressionId dest;
    if (!funcCall->dest().empty()) {
      const PlexilExprId& destExpr = funcCall->dest()[0]->getId();
      if (Id<PlexilVarRef>::convertable(destExpr)) {
        dest = findVariable(destExpr);
        checkError(dest.isValid(),
                   "Unknown destination variable '" << destExpr->name() <<
                   "' in function call '" << name.toString() << "' in node '" <<
                   m_nodeId.toString() << "'");
      }
      else if (Id<PlexilVarRef>::convertable(destExpr)) {
        dest = ExpressionFactory::createInstance(destExpr->name(),
                                                 destExpr,
                                                 m_connector);
        garbage.push_back(dest);
      }
      else {
        checkError(ALWAYS_FAIL, "Invalid left-hand side for function call");
      }

    }

    debugMsg("Node:createFunctionCall",
	     "Creating function call '" << name.toString() << "' for node '" <<
	     m_nodeId.toString() << "'");
    m_functionCall = (new FunctionCall(nameExpr, args, dest, m_ack, garbage))->getId();
    check_error(m_functionCall.isValid());
  }


  ExpressionId& Node::getCondition(const LabelStr& name) {
    std::map<double, ExpressionId>::iterator it = m_conditionsByName.find(name);
    checkError(it != m_conditionsByName.end(),
	       "No condition '" << name.toString() << "' in node '" << m_nodeId.toString() <<
	       "'");
    return it->second;
  }

  double Node::getAcknowledgementValue() const 
  {
    return ((Variable*)m_ack)->getValue();
  }

  void Node::createConditions(const std::map<std::string, PlexilExprId>& conds) {
    if(m_parent.isId()) {
      ExpressionId ancestorInvariant =
	(new Conjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_INVARIANT_CONDITION())))->getId(),
			 true,
			 (new TransparentWrapper(m_parent->getCondition(INVARIANT_CONDITION())))->getId(),
			 true))->getId();
      ExpressionId ancestorEnd =
         (new Disjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_END_CONDITION())))->getId(),
                          true,
                          (new TransparentWrapper(m_parent->getCondition(END_CONDITION())))->getId(),
                          true))->getId();
      ExpressionId ancestorExecuting =
	(new Equality(m_parent->getStateVariable(),
		      StateVariable::EXECUTING_EXP()))->getId();
      ExpressionId parentWaiting =
	(new Equality(m_parent->getStateVariable(),
		      StateVariable::WAITING_EXP()))->getId();
      ExpressionId parentFinished =
	(new Equality(m_parent->getStateVariable(),
		      StateVariable::FINISHED_EXP()))->getId();

      ExpressionListenerId ancestorInvariantListener =
	m_listenersByName[ANCESTOR_INVARIANT_CONDITION()];
//       if(!ancestorInvariantListener->isActive())
// 	ancestorInvariantListener->activate();
      ExpressionListenerId ancestorEndListener =
	m_listenersByName[ANCESTOR_END_CONDITION()];
//       if(!ancestorEndListener->isActive())
// 	ancestorEndListener->activate();
      ExpressionListenerId ancestorExecutingListener =
	m_listenersByName[PARENT_EXECUTING_CONDITION()];
      if(!ancestorExecutingListener->isActive())
	ancestorExecutingListener->activate();
      ExpressionListenerId parentWaitingListener =
	m_listenersByName[PARENT_WAITING_CONDITION()];
      ExpressionListenerId parentFinishedListener =
	m_listenersByName[PARENT_FINISHED_CONDITION()];
      if(!parentFinishedListener->isActive())
	parentFinishedListener->activate();

      m_conditionsByName[ANCESTOR_INVARIANT_CONDITION()]->removeListener(ancestorInvariantListener);
      delete (Expression*) m_conditionsByName[ANCESTOR_INVARIANT_CONDITION()];

      //ancestorInvariant->activate();
      ancestorInvariant->addListener(ancestorInvariantListener);

      m_conditionsByName[ANCESTOR_END_CONDITION()]->removeListener(ancestorEndListener);
      delete (Expression*) m_conditionsByName[ANCESTOR_END_CONDITION()];

      //ancestorEnd->activate();
      ancestorEnd->addListener(ancestorEndListener);

      m_conditionsByName[PARENT_EXECUTING_CONDITION()]->removeListener(ancestorExecutingListener);
      delete (Expression*) m_conditionsByName[PARENT_EXECUTING_CONDITION()];

      ancestorExecuting->activate(); //activate this right off so we can start executing
      ancestorExecuting->addListener(ancestorExecutingListener);

      m_conditionsByName[PARENT_WAITING_CONDITION()]->removeListener(parentWaitingListener);
      delete (Expression*) m_conditionsByName[PARENT_WAITING_CONDITION()];
      parentWaiting->addListener(parentWaitingListener);

      m_conditionsByName[PARENT_FINISHED_CONDITION()]->removeListener(parentFinishedListener);
      delete (Expression*) m_conditionsByName[PARENT_FINISHED_CONDITION()];
      parentFinished->activate();
      parentFinished->addListener(parentFinishedListener);

      m_conditionsByName[ANCESTOR_INVARIANT_CONDITION()] = ancestorInvariant;
      m_conditionsByName[ANCESTOR_END_CONDITION()] = ancestorEnd;
      m_conditionsByName[PARENT_EXECUTING_CONDITION()] = ancestorExecuting;
      m_conditionsByName[PARENT_WAITING_CONDITION()] = parentWaiting;
      m_conditionsByName[PARENT_FINISHED_CONDITION()] = parentFinished;

      m_garbage.insert(ANCESTOR_INVARIANT_CONDITION());
      m_garbage.insert(PARENT_EXECUTING_CONDITION());
      m_garbage.insert(PARENT_WAITING_CONDITION());
      m_garbage.insert(PARENT_FINISHED_CONDITION());
      m_garbage.insert(ANCESTOR_END_CONDITION());
    }

    for(std::map<std::string, PlexilExprId>::const_iterator it = conds.begin(); 
	it != conds.end(); ++it) {
      LabelStr condName(it->first);
      ExpressionId expr = ExpressionId::noId();
      if(Id<PlexilVarRef>::convertable(it->second)) {
	expr = findVariable(it->second);
      }
      else {
	expr = ExpressionFactory::createInstance(it->second->name(), it->second,
						 m_connector);
	m_garbage.insert(condName);
      }
      ExpressionListenerId condListener = m_listenersByName[condName];
      
      m_conditionsByName[condName]->removeListener(condListener);
      delete (Expression*) m_conditionsByName[condName];
      m_conditionsByName[condName] = expr;
      expr->addListener(condListener);
    }

    if(m_nodeType == COMMAND() || m_nodeType == ASSIGNMENT() || m_nodeType == UPDATE() || m_nodeType == FUNCTION()) {
      if(m_nodeType == COMMAND()) 
        {
          ExpressionId commandAbort = (new BooleanVariable())->getId();
          ExpressionListenerId abortListener = m_listenersByName[ABORT_COMPLETE()];
          
          commandAbort->addListener(abortListener);
          m_conditionsByName[ABORT_COMPLETE()] = commandAbort;
          m_ack = (new StringVariable(StringVariable::UNKNOWN()))->getId();
          
          m_conditionsByName[END_CONDITION()]->removeListener(m_listenersByName[END_CONDITION()]);
          
          m_extraEndCond = (new IsKnown(m_ack))->getId();
          m_interruptEndCond = (new InterruptibleCommandHandleValues(m_ack))->getId();
          
          m_conjunctCondition = (new Conjunction(m_extraEndCond, false, 
                                                 m_conditionsByName[END_CONDITION()],
                                                 m_garbage.find(END_CONDITION()) != m_garbage.end()))->getId();
          ExpressionId realEndCondition =
            (new Disjunction(m_interruptEndCond, false, 
                             m_conjunctCondition, m_garbage.find(END_CONDITION()) != m_garbage.end()))->getId();
          
          realEndCondition->addListener(m_listenersByName[END_CONDITION()]);
          m_conditionsByName[END_CONDITION()] = realEndCondition;
          m_garbage.insert(END_CONDITION());
          
          // Listen to any change in the command handle so that the internal variable 
          // CommandHandleVariable can be updated
          m_conditionsByName[COMMAND_HANDLE_RECEIVED_CONDITION()]->removeListener(m_listenersByName[COMMAND_HANDLE_RECEIVED_CONDITION()]);
          m_allCommandHandleValues = (new AllCommandHandleValues(m_ack))->getId();
          m_allCommandHandleValues->ignoreCachedValue();
          ExpressionId realCmdHandleCondition =
            (new Conjunction(m_allCommandHandleValues, false, m_conditionsByName[COMMAND_HANDLE_RECEIVED_CONDITION()],
                             m_garbage.find(COMMAND_HANDLE_RECEIVED_CONDITION()) != m_garbage.end()))->getId();
          realCmdHandleCondition->addListener(m_listenersByName[COMMAND_HANDLE_RECEIVED_CONDITION()]);
          m_conditionsByName[COMMAND_HANDLE_RECEIVED_CONDITION()] = realCmdHandleCondition;
          m_conditionsByName[COMMAND_HANDLE_RECEIVED_CONDITION()]->ignoreCachedValue();
          m_garbage.insert(COMMAND_HANDLE_RECEIVED_CONDITION());
        }
      else
        {
          m_conditionsByName[END_CONDITION()]->removeListener(m_listenersByName[END_CONDITION()]);
          m_ack = (new BooleanVariable(BooleanVariable::UNKNOWN()))->getId();
          ExpressionId realEndCondition =
            (new Conjunction(m_ack, false, m_conditionsByName[END_CONDITION()],
                             m_garbage.find(END_CONDITION()) != m_garbage.end()))->getId();
          realEndCondition->addListener(m_listenersByName[END_CONDITION()]);
          m_conditionsByName[END_CONDITION()] = realEndCondition;
          m_garbage.insert(END_CONDITION());
        }
    }
  }

  void Node::createChildNodes(const PlexilListBody* body) {
    checkError(m_nodeType == LIST(), "Attempted to create child nodes for a non-list node.");
    for(std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
	it != body->children().end(); ++it)
      m_children.push_back((new Node(*it, m_exec, m_id))->getId());

    ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
    ExpressionListenerId listener = m_listenersByName[CHILDREN_WAITING_OR_FINISHED()];
    cond->addListener(listener);
    m_conditionsByName[CHILDREN_WAITING_OR_FINISHED()] = cond;
    m_garbage.insert(CHILDREN_WAITING_OR_FINISHED());

    ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
    listener = m_listenersByName[END_CONDITION()];
    endCond->addListener(listener);
    m_conditionsByName[END_CONDITION()] = endCond;
    m_garbage.insert(END_CONDITION());
  }

  // Check aliases against interfaceVars.
  // Remove all that are found from aliases.
  // If a variable exists in interfaceVars but not aliases:
  //  - and it has a default value, generate the variable with the default value;
  //  - and it doesn't have a default value, signal an error.
  // libNode is only used for error message generation.

   void Node::testLibraryNodeParameters(
      const PlexilNodeId& libNode, 
      const std::vector<PlexilVarRef*>& interfaceVars,
      PlexilAliasMap& aliases)
   {
      // check each variable in the interface to ensure it is
      // referenced in the alias list
      
      for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
           var != interfaceVars.end(); ++var)
      {
	// get var label and matching value in alias list
	LabelStr varLabel((*var)->name());
	PlexilExprId& aliasValue = aliases[varLabel];

	// check that variable referenced in alias list         
	// if it is missing then check whether formal parameter has a default value
	if (!aliasValue.isId())
	  {
	    if ((*var)->defaultValue().isId())
	      {
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
		m_variablesByName[varLabel] = 
		  ExpressionFactory::createInstance(PlexilParser::valueTypeString((*var)->type()) + "Value",
						    defaultValue->getId(),
						    m_connector);
	      }
	    else
	      {
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
      
      // get the lib node and it's interface
      
      const PlexilNodeId& libNode = body->libNode();
      const PlexilInterfaceId& libInterface = libNode->interface();
      
      // if there is no interface, there must be no variables
      
      if (libInterface.isId() == false)
      {
         checkError(body->aliases().size() == 0,
                    "Variable aliases in '" << getNodeId().toString() <<
                    "' do not match interface in '" << 
                    libNode->nodeId() << "'");
      }
      
      // otherwise check variables in interface

      else
      {
         // make a copy of the alias map

         PlexilAliasMap aliasesCopy(body->aliases());

         // check each "In" variable in the interface to ensure it is
         // referenced in the alias list
         
         testLibraryNodeParameters(libNode, libInterface->in(), aliasesCopy);

         // check each "InOut" variable in the interface to ensure it is
         // referenced in the alias list
         
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
           alias != body->aliases().end(); ++alias)
      {
         LabelStr paramName(alias->first);
         
         // if this is a variable reference, look it up
         
         if (Id<PlexilVarRef>::convertable(alias->second))
         {
            const PlexilVarRef* paramVar = alias->second;
            
            // find variable in interface
            
            const PlexilVarRef* iVar = libInterface->findVar(paramName.toString());
            
            // be sure it exists in interface
            
            checkError(iVar != NULL,
                       "Variable '" << paramName.toString()
                       << "' referenced in '" << getNodeId().toString()
                       << "' does not appear in interface of '" 
                       << libNode->nodeId() << "' ");
            
            // check type
            
            checkError(iVar->type() == paramVar->type(), 
                       "Variable type mismatch between '" 
                       << iVar->name() << "' (" << iVar->type() << ") and '" 
                       << paramVar->name() << "' (" << paramVar->type() << ") "
                       << "' referenced in '" << getNodeId().toString() << "'");

            // find the expression form

            const Id<Expression>& varExp = findVariable(alias->second);
            checkError(varExp.isId(), "Unknown variable '" 
                       << alias->second->name()
                       << "' referenced in call to '" << libNode->nodeId() << "' from '"
                       << getNodeId().toString() << "'");
            checkError(Id<Variable>::convertable(varExp), "Expression not a variable '" 
                       << alias->second->name()
                       << "' referenced in call to '" << libNode->nodeId() << "' from '"
                       << getNodeId().toString() << "'");

            // covert expression to variable

            const Id<Variable>& var = (const Id<Variable>&)varExp;

            // check that read only variables appear in the the In interface
            
            checkError(!var->isConst() || libInterface->findInVar(paramName.toString()),
                       "Constant variable '" << alias->second->name()
                       << "' referenced in '" << getNodeId().toString() 
                       << "' is aliased to '" <<  iVar->name()
                       << "' declaried as InOut in '" << libNode->nodeId() << "'");

            // add this variable to node

            m_variablesByName[paramName] = var;
         }
         
            // if this is a value, create a local variable for it
         
         else if (Id<PlexilValue>::convertable(alias->second))
         {
            Id<PlexilValue> value = (Id<PlexilValue>)alias->second;
            debugMsg("Node:createLibraryNode",
                     " Constructing variable for " << value->name()
                     << " literal with value " << value->value() 
                     << " as library node interface variable " << LabelStr(paramName).c_str());
            ExpressionId varId = 
              ExpressionFactory::createInstance(value->name(),
                                                value->getId(),
                                                m_connector);
            m_variablesByName[paramName] = varId;
         }
         else
            checkError(false, 
                       "Unexpected expression type '" << alias->second->name()
                       << "' in: " << getNodeId().toString());
      }
      
      m_children.push_back((new Node(body->libNode(), m_exec, m_id))->getId());

      ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
      ExpressionListenerId listener = m_listenersByName[CHILDREN_WAITING_OR_FINISHED()];
      cond->addListener(listener);
      m_conditionsByName[CHILDREN_WAITING_OR_FINISHED()] = cond;
      m_garbage.insert(CHILDREN_WAITING_OR_FINISHED());

      ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
      listener = m_listenersByName[END_CONDITION()];
      endCond->addListener(listener);
      m_conditionsByName[END_CONDITION()] = endCond;
      m_garbage.insert(END_CONDITION());    
   }

   // extract variable from parent which are not already present in this node

   void Node::getVarsFromParent()
   {
         // if there is no a parent, we're done
    
      if (!m_parent.isId())
         return;

         // debug message

      debugMsg("Node:getVarsFromParent", "Import variables from parent '"
               << m_parent->m_nodeId.toString()
               << "' to node '"
               << m_nodeId.toString() << "'");

         // have a look at the variables in the parent

      ExpressionMap& parentVars = m_parent->m_variablesByName;
      for (ExpressionMap::const_iterator parentVar = parentVars.begin();
           parentVar != parentVars.end(); ++parentVar)
      {
         LabelStr parentVarName(parentVar->first);

            // if the variable does not exist locally, add it
            // otherwise ignore it, locals mask

         if (m_variablesByName.find(parentVarName) == m_variablesByName.end())
            m_variablesByName[parentVarName] = parentVar->second;
      }
   }

   void Node::getVarsFromInterface(const PlexilInterfaceId& intf)
   {
      if(!intf.isValid())
         return;
      
      debugMsg("Node:getVarsFromInterface",
               "Getting interface vars for node '" << m_nodeId.toString() << "'");
      checkError(m_parent.isId(), "Bizarre.  An interface on a parentless node.");
      //CHECK FOR DUPLICATE NAMES this is the point at which we may want
      //a ConstantWrapper class to enforce "in" semantics
      for(std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
          it != intf->in().end(); ++it) 
      {
         ExpressionId expr = m_parent->findVariable(*it);
         checkError(expr.isId(),
                    "No variable named '" << (*it)->name() <<
                    "' in parent of node '" << m_nodeId.toString() << "'");
         checkError(Id<Variable>::convertable(expr),
                    "Expression named '" << (*it)->name() <<
                    "' in parent of node '" << m_nodeId.toString() <<
                    "' not a variable.");
         
         // if variable present in the In interface, it is constant
         
         if (m_node->interface()->findInVar(*it))
            ((Id<Variable>)expr)->setConst();
         
         // add variable to this node
         
         m_variablesByName[LabelStr((*it)->name())] = expr;
      }
      
      for(std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
          it != intf->inOut().end(); ++it) 
      {
         ExpressionId expr = m_parent->findVariable(*it);
         checkError(expr.isId(),
                    "No variable named '" << (*it)->name() <<
                    "' in parent of node '" << m_nodeId.toString() << "'");
         
         m_variablesByName[LabelStr((*it)->name())] = expr;
      }
   }

  void Node::createDeclaredVars(const std::vector<PlexilVarId>& vars) {
    for(std::vector<PlexilVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
       // get the variable name

       LabelStr name((*it)->name());

       // if it's an array, make me an array
       
       if (Id<PlexilArrayVar>::convertable((*it)->getId()))
       {
          PlexilValue* value = (*it)->value();
          ExpressionId varId = ExpressionFactory::createInstance(value->name(), value->getId(),
                                                                 m_connector);
          //CHECK FOR DUPLICATE NAMES

          m_variablesByName[name] = varId;
          m_localVariables.push_back(varId);
          debugMsg("Node:createDeclaredVars",
                   "From node '" << m_nodeId.toString() << "', created variable " <<
                   m_variablesByName[name]->toString());
       }
       // otherwise create a non-array variable

       else
       {
          PlexilValue* value = (*it)->value();
          ExpressionId varId = ExpressionFactory::createInstance(value->name(), value->getId(),
                                                                 m_connector);
          //CHECK FOR DUPLICATE NAMES

          m_variablesByName[name] = varId;
          m_localVariables.push_back(varId);
          debugMsg("Node:createDeclaredVars",
                   "From node '" << m_nodeId.toString() << "', created variable " <<
                   m_variablesByName[name]->toString());
       }
    }
  }

  void Node::checkConditions() {
    if(m_transitioning)
      return;

    debugMsg("Node:checkConditions",
	     "Checking condition change for node " << m_nodeId.toString());
    LabelStr toState = getDestState();
    debugMsg("Node:checkConditions",
	     "Can (possibly) transition to " << toState.toString());
    if(toState != m_lastQuery) {
      if((toState != StateVariable::UNKNOWN() && toState != StateVariable::NO_STATE()) ||
	 ((toState == StateVariable::UNKNOWN() || toState == StateVariable::NO_STATE()) &&
	  (m_lastQuery != StateVariable::UNKNOWN() &&
	   m_lastQuery != StateVariable::NO_STATE())))
	m_exec->handleConditionsChanged(m_id);
      m_lastQuery = toState;
    }
  }

  void Node::transition(const double time) {
    checkError(!m_transitioning,
	       "Node " << m_nodeId.toString() << " is already transitioning.");
    m_transitioning = true;
    LabelStr prevState = getState();
    NodeStateManager::getStateManager(getType())->transition(m_id);
    debugMsg("Node:transition", "Transitioning '" << m_nodeId.toString() <<
	     "' from " << prevState.toString() << " to " << getState().toString());
    condDebugMsg((getState() == StateVariable::FINISHED()),
                 "Node:outcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    condDebugMsg((getState() == StateVariable::ITERATION_ENDED()),
                 "Node:iterationOutcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    debugMsg("Node:times",
	     "Setting end time " << (prevState.toString() + ".END") << " = " << time);
    debugMsg("Node:times",
	     "Setting start time " << (getState().toString() + ".START") << " = " << time);
    m_variablesByName[LabelStr(prevState.toString() + ".END")]->setValue(time);
    m_variablesByName[LabelStr(getState().toString() + ".START")]->setValue(time);
    m_transitioning = false;
    checkConditions();
  }

  const ExpressionId& Node::getInternalVariable(const LabelStr& name) {
    checkError(m_variablesByName.find(name) != m_variablesByName.end(),
	       "No variable named " << name.toString() << " in " << m_nodeId.toString());
    return m_variablesByName.find(name)->second;
  }

  const LabelStr Node::getState() {
    return getStateVariable()->getValue();
  }

  const ExpressionId& Node::getStateVariable() {
    return getInternalVariable(STATE());
  }

  const LabelStr Node::getOutcome() {
    return getOutcomeVariable()->getValue();
  }

  const ExpressionId& Node::getOutcomeVariable() {
    return getInternalVariable(OUTCOME());
  }

  const LabelStr Node::getFailureType() {
    return getFailureTypeVariable()->getValue();
  }

  const ExpressionId& Node::getFailureTypeVariable() {
    return getInternalVariable(FAILURE_TYPE());
  }

  const LabelStr Node::getCommandHandle() {
    return getCommandHandleVariable()->getValue();
  }

  const ExpressionId& Node::getCommandHandleVariable() {
    return getInternalVariable(COMMAND_HANDLE());
  }

  class NodeIdEq {
  public:
    NodeIdEq(const double name) : m_name(name) {}
    bool operator()(const NodeId& node) {return node->getNodeId() == m_name;}
  private:
    double m_name;
  };

  const ExpressionId& Node::findVariable(const LabelStr& name) const
  {
    debugMsg("Node:findVariable",
	     " Searching for variable \"" << name.toString() << "\" in node " <<
	     m_nodeId.toString());
    std::map<double, ExpressionId>::const_iterator it = m_variablesByName.find(name);
    checkError(it != m_variablesByName.end(),
	       "No variable named \"" << name.toString() << "\" in node " <<
	       m_nodeId.toString());
    return (it == m_variablesByName.end() ? ExpressionId::noId() : it->second);
  }
   
   const ExpressionId& Node::findVariable(const PlexilVarRef* ref) const
   {
      debugMsg("Node:findVariable",
               " Searching for variable reference \"" << ref->name() << 
               "\" in node " << m_nodeId.toString());
      
      if(Id<PlexilInternalVar>::convertable(ref->getId())) 
      {
         PlexilInternalVar* var = (PlexilInternalVar*) ref;
         PlexilNodeRef* nodeRef = var->ref();
         NodeId node = NodeId::noId();
         
         switch(nodeRef->dir()) 
         {
            case PlexilNodeRef::SELF:
               node = m_id;
               break;
            case PlexilNodeRef::PARENT:
               checkError(m_parent.isValid(),
                          "Parent node reference in root node " << 
                          m_nodeId.toString());
               node = m_parent;
               break;
            case PlexilNodeRef::CHILD:
            {
               checkError(m_nodeType == Node::LIST(),
                          "Child internal variable reference in node " << 
                          m_nodeId.toString() <<
                          " which isn't a list node.");
               std::list<NodeId>::const_iterator it =
                  std::find_if(m_children.begin(), m_children.end(),
                               NodeIdEq(LabelStr(nodeRef->name())));
               checkError(it != m_children.end(),
                          "No child named '" << nodeRef->name() << 
                          "' in " << m_nodeId.toString());
               node = *it;
               break;
            }
            case PlexilNodeRef::SIBLING: 
            {
               checkError(m_parent.isValid(),
                          "Parent node reference in root node " << 
                          m_nodeId.toString());
               std::list<NodeId>::const_iterator it =
                  std::find_if(m_parent->m_children.begin(), 
                               m_parent->m_children.end(),
                               NodeIdEq(LabelStr(nodeRef->name())));
               checkError(it != m_parent->m_children.end(),
                          "No sibling named '" << nodeRef->name() << 
                          "' of " << m_nodeId.toString());
               node = *it;
               break;
            }
            case PlexilNodeRef::NO_DIR:
            default:
               checkError(ALWAYS_FAIL,
                          "Invalid direction in node reference from " <<
                          m_nodeId.toString());
               return ExpressionId::noId();
         }
         std::string name;
         if(Id<PlexilTimepointVar>::convertable(var->getId())) 
         {
            PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
            name = tp->state() + "." + tp->timepoint();
         }
         else
            name = var->name();
         debugMsg("Node:findVariable", 
                  " Found internal variable \"" << name << "\"");
         return node->getInternalVariable(LabelStr(name));
      }
      else 
      {
         std::map<double, ExpressionId>::const_iterator it =
            m_variablesByName.find(LabelStr(ref->name()));
         
         checkError(it != m_variablesByName.end(),
                    "Can't find variable " << ref->name() << 
                    " in node " << m_nodeId.toString());
         if (it == m_variablesByName.end())
           {
             debugMsg("Node:findVariable", " not found, returning noId()");
             return ExpressionId::noId();
           }
         debugMsg("Node:findVariable",
                  " Returning regular variable " << it->second->toString());
         return it->second;
      }
      return ExpressionId::noId();
  }

//   const ExpressionId& Node::_findVariable(const PlexilVarId& ref) 
//   {
//      debugMsg("Node:findVariable",
//               "Searching for variable '" << var->name() << " in node "
//               << m_nodeId.toString());
     
//      if (Id<PlexilInternalVar>::convertable(ref->getId())) 
//      {
//         PlexilInternalVar* var = (PlexilInternalVar*) ref;
//         PlexilNodeRef* nodeRef = var->ref();
//         NodeId node = NodeId::noId();

//       switch(nodeRef->dir()) {
//       case PlexilNodeRef::SELF:
// 	node = m_id;
// 	break;
//       case PlexilNodeRef::PARENT:
// 	checkError(m_parent.isValid(),
// 		   "Parent node reference in root node " << m_nodeId.toString());
// 	node = m_parent;
// 	break;
//       case PlexilNodeRef::CHILD: {
// 	checkError(m_nodeType == Node::LIST(),
// 		   "Child internal variable reference in node " << m_nodeId.toString() <<
// 		   " which isn't a list node.");
// 	std::list<NodeId>::iterator it =
// 	  std::find_if(m_children.begin(), m_children.end(),
// 		       NodeIdEq(LabelStr(nodeRef->name())));
// 	checkError(it != m_children.end(),
// 		   "No child named '" << nodeRef->name() << "' in " << m_nodeId.toString());
// 	node = *it;
// 	break;
//       }
//       case PlexilNodeRef::SIBLING: {
// 	checkError(m_parent.isValid(),
// 		   "Parent node reference in root node " << m_nodeId.toString());
// 	std::list<NodeId>::iterator it =
// 	  std::find_if(m_parent->m_children.begin(), m_parent->m_children.end(),
// 		       NodeIdEq(LabelStr(nodeRef->name())));
// 	checkError(it != m_parent->m_children.end(),
// 		   "No sibling named '" << nodeRef->name() << "' of " << m_nodeId.toString());
// 	node = *it;
// 	break;
//       }
//       case PlexilNodeRef::NO_DIR:
//       default:
// 	checkError(ALWAYS_FAIL,
// 		   "Invalid direction in node reference from " << m_nodeId.toString());
// 	return ExpressionId::noId();
//       }
//       std::string name;
//       if(Id<PlexilTimepointVar>::convertable(var->getId())) {
// 	PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
// 	name = tp->state() + "." + tp->timepoint();
//       }
//       else
// 	name = var->name();
//       return node->getInternalVariable(LabelStr(name));
//     }
//     else {
//       std::map<double, ExpressionId>::iterator it =
// 	m_variablesByName.find(LabelStr(ref->name()));
//       checkError(it != m_variablesByName.end(),
// 		 "Can't find variable " << ref->name() << " in node " << m_nodeId.toString());
//       return (it == m_variablesByName.end() ? ExpressionId::noId() : it->second);
//     }
//     return ExpressionId::noId();
//   }
  //cheesy hack?
  CommandId& Node::getCommand() {
    if(getState() == StateVariable::EXECUTING())
      m_command->activate();
    if(m_command.isValid()) {
      m_command->fixValues();
      m_command->fixResourceValues();
    }
    return m_command;
  }

  UpdateId& Node::getUpdate() {
    if(getState() == StateVariable::EXECUTING())
      m_update->activate();
    if(m_update.isValid())
      m_update->fixValues();
    return m_update;
  }

  FunctionCallId& Node::getFunctionCall() {
    if(getState() == StateVariable::EXECUTING())
      m_functionCall->activate();
    if(m_functionCall.isValid()) {
      m_functionCall->fixValues();
    }
    return m_functionCall;
  }

  const ExpressionId& Node::getAssignmentVariable() const {return m_assignment->getDest();}

  AssignmentId& Node::getAssignment() {
    check_error(getType() == ASSIGNMENT());
    if(getState() == StateVariable::EXECUTING())
      m_assignment->activate();
    m_assignment->fixValue();
    return m_assignment;
  }

  const LabelStr& Node::getDestState() {
    debugMsg("Node:getDestState",
	     "Getting destination state for " << m_nodeId.toString() << " from state " <<
	     getState().toString());
    return NodeStateManager::getStateManager(getType())->getDestState(m_id);
  }

  void Node::activatePair(const LabelStr& name) {
    checkError(m_listenersByName.find(name) != m_listenersByName.end() &&
	       m_conditionsByName.find(name) != m_conditionsByName.end(),
	       "No condition/listener pair exists for '" << name.toString() << "'");
    debugMsg("Node:activatePair",
	     "Activating '" << name.toString() << "' in node '" << m_nodeId.toString());
    m_listenersByName[name]->activate();
    m_conditionsByName[name]->activate();
  }

  void Node::deactivatePair(const LabelStr& name) {
    checkError(m_listenersByName.find(name) != m_listenersByName.end() &&
	       m_conditionsByName.find(name) != m_conditionsByName.end(),
	       "No condition/listener pair exists for '" << name.toString() << "'");
    debugMsg("Node:deactivatePair",
	     "Deactivating '" << name.toString() << "' in node '" << m_nodeId.toString());
    m_conditionsByName[name]->deactivate();
    if(m_listenersByName[name]->isActive())
      m_listenersByName[name]->deactivate();
  }

  bool Node::pairActive(const LabelStr& name) {
    checkError(m_listenersByName.find(name) != m_listenersByName.end() &&
	       m_conditionsByName.find(name) != m_conditionsByName.end(),
	       "No condition/listener pair exists for '" << name.toString() << "'");
    condDebugMsg(!m_listenersByName[name]->isActive(), "Node:pairActive",
		 "Listener for " << name.toString() << " in " << m_nodeId.toString() <<
		 " is inactive.");
    condDebugMsg(!m_conditionsByName[name]->isActive(), "Node:pairActive",
		 "Condition " << name.toString() << " in " << m_nodeId.toString() <<
		 " is inactive.");
    return m_listenersByName[name]->isActive() && m_conditionsByName[name]->isActive();
  }

  void Node::execute() {
    debugMsg("Node:execute", "Executing node " << m_nodeId.toString());
    // activate local variables
    for (std::list<ExpressionId>::iterator vit = m_localVariables.begin();
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
    ((Variable*)m_variablesByName[OUTCOME()])->reset();
    ((Variable*)m_variablesByName[FAILURE_TYPE()])->reset();
    ((Variable*)m_variablesByName[COMMAND_HANDLE()])->reset();

    std::vector<std::string> suffix;
    suffix.push_back("START");
    suffix.push_back("END");

    //reset timepoints
    for(std::set<double>::const_iterator it = StateVariable::ALL_STATES().begin();
	it != StateVariable::ALL_STATES().end(); ++it) {
      for(unsigned int i = 0; i < suffix.size(); i++) {
	std::stringstream str;
	str << (LabelStr(*it)).toString() << "." << suffix[i];
	LabelStr varName(str.str());
	((Variable*)m_variablesByName[varName])->reset();
      }
    }

    for(std::list<ExpressionId>::const_iterator it = m_localVariables.begin();
	it != m_localVariables.end(); ++it) {
      ((Variable*)(*it))->reset();
    }
    
    if(getType() == COMMAND() || getType() == UPDATE() || getType() == FUNCTION() ||
       getType() == REQUEST())
      ((Variable*)m_ack)->reset();
  }

  void Node::abort() {
    debugMsg("Node:abort", "Aborting node " << m_nodeId.toString());
    if(getType() == Node::COMMAND() && m_command.isValid())
      m_exec->getExternalInterface()->invokeAbort(m_command->getName(),
                                                  m_command->getArgValues(),
                                                  m_conditionsByName[ABORT_COMPLETE()]);
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

  void Node::lockConditions() {
    for(std::map<double, ExpressionId>::iterator it = m_conditionsByName.begin();
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

  void Node::unlockConditions() {
    for(std::map<double, ExpressionId>::iterator it = m_conditionsByName.begin();
	it != m_conditionsByName.end(); ++it) {
      ExpressionId expr = it->second;
      check_error(expr.isValid());
      if(expr->isLocked()) {
	debugMsg("Node:unlockConditions",
		 "In " << m_nodeId.toString() << ", unlocking " <<
		 LabelStr(it->first).toString() << " " << expr->toString());
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
    else if(getType() == Node::FUNCTION() && m_functionCall.isValid())
      m_functionCall->deactivate();
    // deactivate local variables
    for (std::list<ExpressionId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         vit++)
      {
        (*vit)->deactivate();
      }
  }

  std::string Node::toString(const unsigned int indent) {
    std::stringstream indentStr;
    for(unsigned int i = 0; i < indent; i++)
      indentStr << " ";
    std::stringstream retval;

    retval << indentStr.str() << m_nodeId.toString() << "{" << std::endl;
    retval << indentStr.str() << " State: " << m_variablesByName[STATE()]->toString() <<
      " (" <<
      m_variablesByName[LabelStr(LabelStr(m_variablesByName[STATE()]->getValue()).toString() + ".START")]->getValue() <<
      ")" << std::endl;
    if(m_variablesByName[STATE()]->getValue() == StateVariable::FINISHED()) {
      retval << indentStr.str() << " Outcome: " << m_variablesByName[OUTCOME()]->toString() <<
	std::endl;
      if(m_variablesByName[FAILURE_TYPE()]->getValue() != OutcomeVariable::UNKNOWN())
	retval << indentStr.str() << " Failure type: " <<
	  m_variablesByName[FAILURE_TYPE()]->toString() << std::endl;
      if(m_variablesByName[COMMAND_HANDLE()]->getValue() != CommandHandleVariable::UNKNOWN())
	retval << indentStr.str() << " Command handle: " <<
	  m_variablesByName[COMMAND_HANDLE()]->toString() << std::endl;
      for(std::map<double, ExpressionId>::iterator it = m_variablesByName.begin();
	  it != m_variablesByName.end(); ++it) {
	if(it->first == STATE() || it->first == OUTCOME() || it->first == FAILURE_TYPE() 
           || it->first == COMMAND_HANDLE() ||
	   LabelStr(it->first).countElements(".") > 1)
	  continue;
	retval << indentStr.str() << " " << LabelStr(it->first).toString() << ": " <<
	  (*it).second->toString() << std::endl;
      }
    }
    else if(m_variablesByName[STATE()]->getValue() != StateVariable::INACTIVE()) {
      for(std::map<double, ExpressionId>::iterator it = m_conditionsByName.begin();
	  it != m_conditionsByName.end(); ++it) {
	retval << indentStr.str() << " " << LabelStr(it->first).toString() << ": " <<
	  (*it).second->toString() << std::endl;
      }
      for(std::map<double, ExpressionId>::iterator it = m_variablesByName.begin();
	  it != m_variablesByName.end(); ++it) {
	if(it->first == STATE() || it->first == OUTCOME() || it->first == FAILURE_TYPE() 
           || LabelStr(it->first).countElements(".") > 1)
	  continue;
	retval << indentStr.str() << " " << LabelStr(it->first).toString() << ": " <<
	  (*it).second->toString() << std::endl;
      }
    }
    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      retval << (*it)->toString(indent + 2);
    }
    retval << indentStr.str() << "}" << std::endl;
    return retval.str();
  }

  Command::Command(const ExpressionId nameExpr, const std::list<ExpressionId>& args,
		   const ExpressionId dest, const ExpressionId ack,
		   const std::list<ExpressionId>& garbage,
                   const std::vector<std::map<std::string, ExpressionId> >& resource)
    : m_id(this), m_nameExpr(nameExpr), m_args(args), m_dest(dest), m_ack(ack), 
      m_garbage(garbage), m_resourceList(resource) {}

  Command::~Command() {
    for(std::list<ExpressionId>::const_iterator it = m_garbage.begin();
	it != m_garbage.end(); ++it)
      delete (Expression*) (*it);
    m_nameExpr.remove();
    m_id.remove();
  }

   const LabelStr& Command::getName() 
   {
      return m_name = LabelStr(m_nameExpr->getValue());
   }

  void Command::fixValues() {
    m_argValues.clear();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      m_argValues.push_back(expr->getValue());
    }
  }

  void Command::fixResourceValues()
  {
    m_resourceValues.clear();
    for(std::vector<std::map<std::string, ExpressionId> >::const_iterator resListIter = 
          m_resourceList.begin(); resListIter != m_resourceList.end(); ++resListIter)
      {
        std::map<std::string, double> resValues;
        for(std::map<std::string, ExpressionId>::const_iterator resIter = resListIter->begin();
            resIter != resListIter->end(); ++resIter)
          {
            ExpressionId expr = resIter->second;
            check_error(expr.isValid());
            resValues[resIter->first] = expr->getValue();
            /*            
            std::cout << "Command::fixResourceValues: " << resIter->first;
            if (resIter->first == RESOURCENAME_TAG)
              std:: cout << ": " << LabelStr(resValues[resIter->first]).toString() << std::endl;
            else
                std:: cout << ": " << resValues[resIter->first] << std::endl;
            */
          }
        
        //        std::cout << std::endl;
        m_resourceValues.push_back(resValues);
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
    for(std::vector<std::map<std::string, ExpressionId> >::const_iterator resListIter = 
          m_resourceList.begin(); resListIter != m_resourceList.end(); ++resListIter)
      {
        for(std::map<std::string, ExpressionId>::const_iterator resIter = resListIter->begin();
            resIter != resListIter->end(); ++resIter)
          {
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

  Assignment::Assignment(const ExpressionId lhs, const ExpressionId rhs,
			 const ExpressionId ack, const LabelStr& lhsName, 
                         const bool deleteLhs, const bool deleteRhs)
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

  Update::Update(const NodeId& node, const std::map<double, ExpressionId>& pairs,
		 const ExpressionId ack, const std::list<ExpressionId>& garbage)
    : m_id(this), m_source(node), m_pairs(pairs), m_ack(ack), m_garbage(garbage) {}

  Update::~Update() {
    for(std::list<ExpressionId>::const_iterator it = m_garbage.begin(); it != m_garbage.end();
	++it)
      delete (Expression*) (*it);
    m_id.remove();
  }

  void Update::fixValues() {
    for(std::map<double, ExpressionId>::iterator it = m_pairs.begin(); it != m_pairs.end();
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
    for(std::map<double, ExpressionId>::iterator it = m_pairs.begin(); it != m_pairs.end();
	++it) {
      it->second->activate();
    }
  }

  void Update::deactivate() {
    for(std::map<double, ExpressionId>::iterator it = m_pairs.begin(); it != m_pairs.end();
	++it) {
      it->second->deactivate();
    }
  }

  FunctionCall::FunctionCall(const ExpressionId nameExpr, const std::list<ExpressionId>& args,
                             const ExpressionId dest, const ExpressionId ack,
                             const std::list<ExpressionId>& garbage)
    : m_id(this), m_nameExpr(nameExpr), m_args(args), m_dest(dest), m_ack(ack), m_garbage(garbage) {}

  FunctionCall::~FunctionCall() {
    for(std::list<ExpressionId>::const_iterator it = m_garbage.begin();
	it != m_garbage.end(); ++it)
      delete (Expression*) (*it);
    m_nameExpr.remove();
    m_id.remove();
  }

  void FunctionCall::fixValues() {
    m_argValues.clear();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      m_argValues.push_back(expr->getValue());
    }
  }

  const LabelStr& FunctionCall::getName()
   {
      return m_name = LabelStr(m_nameExpr->getValue());
   }

  //more error checking here
  void FunctionCall::activate() {
    m_nameExpr->activate();
    if(m_dest != ExpressionId::noId())
      m_dest->activate();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
  }

  void FunctionCall::deactivate() {
    m_nameExpr->deactivate();
    if(m_dest != ExpressionId::noId())
      m_dest->deactivate();
    for(std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
  }
}
