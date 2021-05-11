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

#include "AdapterConfiguration.hh"
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"
#include "Array.hh"
#include "Command.hh"
#include "CommandHandler.hh"
#include "Configuration.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecListener.hh"
#include "InterfaceAdapter.hh"
#include "Node.hh"
#include "NodeTransition.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilExec.hh" // g_exec
#include "State.hh"

#include <algorithm> // std::find_if()

namespace PLEXIL
{

  //
  // Constants
  //
  static constexpr const char START_PLAN_CMD[] = "StartPlan";
  static constexpr const char EXIT_PLAN_CMD[] = "ExitPlan";

  static constexpr const char PLAN_STATE_STATE[] = "PlanState";
  static constexpr const char PLAN_OUTCOME_STATE[] = "PlanOutcome";
  static constexpr const char PLAN_FAILURE_TYPE_STATE[] = "PlanFailureType";

  /**
   * @class LauncherListener
   * @brief Helper class to allow plans to monitor other plans
   */
  
  class LauncherListener : public ExecListener
  {
  public:
    LauncherListener(AdapterExecInterface *intf)
      : ExecListener(),
        m_interface(intf)
    {
    }

    virtual ~LauncherListener() = default;

    //! Wrapper method to ensure we don't notify the Exec too often.
    virtual void
    implementNotifyNodeTransitions(std::vector<NodeTransition> const &transitions) const override
    {
      bool notify = false;
      for (NodeTransition const &t : transitions) {
        // We only care about root nodes
        if (t.node->getParent())
          continue;

        // Report a root node transition
        Node const *node = t.node;
        NodeState newState = t.newState;
        Value const nodeIdValue(node->getNodeId());
        debugMsg("LauncherListener:notify",
                 ' ' << node->getNodeId() << " -> " << nodeStateName(newState));

        // Report the node state change
        m_interface->handleValueChange(State(PLAN_STATE_STATE, nodeIdValue),
                                       Value(nodeStateName(newState)));

        NodeOutcome o = node->getOutcome();
        if (o != NO_OUTCOME) {
          // Report the outcome
          debugMsg("LauncherListener:notify",
                   ' ' << node->getNodeId() << " outcome " << outcomeName(o));
          m_interface->handleValueChange(State(PLAN_OUTCOME_STATE, nodeIdValue),
                                         Value(outcomeName(o)));
          FailureType f = node->getFailureType();
          if (f != NO_FAILURE) {
            // Report the failure type
            debugMsg("LauncherListener:notify",
                     ' ' << node->getNodeId() << " failure " << failureTypeName(f));
            m_interface->handleValueChange(State(PLAN_FAILURE_TYPE_STATE, nodeIdValue),
                                           Value(failureTypeName(f)));
          }
        }

        // Be sure to wake the exec
        notify = true;
      }

      // Notify if any root node has changed state.
      if (notify)
        m_interface->notifyOfExternalEvent();
    }

  private:
    AdapterExecInterface *m_interface;
  }; // class LauncherListener

  //
  // Helper functions
  //

  static void valueToExprXml(pugi::xml_node parent, Value const &v)
  {
    ValueType vt = v.valueType();
    if (isArrayType(vt)) {
      pugi::xml_node aryxml = parent.append_child("ArrayValue");
      char const *eltType = typeNameAsValue(arrayElementType(vt));
      aryxml.append_attribute("Type").set_value(eltType);
      Array const *ary;
      v.getValuePointer(ary); // better succeed!
      for (size_t i = 0; i < ary->size(); ++i) {
        // quick & dirty
        aryxml.append_child(eltType).append_child(pugi::node_pcdata)
          .set_value(ary->getElementValue(i).valueToString().c_str());
      }
    }
    else {
      // Scalar value
      parent.append_child(typeNameAsValue(vt)).append_child(pugi::node_pcdata)
        .set_value(v.valueToString().c_str());
    }
  }

  static unsigned int nextSerialNumber()
  {
    static unsigned int sl_next = 0;
    return ++sl_next;
  }

  static bool checkPlanNameArgument(Command *cmd)
  {
    std::vector<Value> const &args = cmd->getArgValues();
    size_t nargs = args.size();
    if (nargs < 1) {
      warn("Not enough parameters to " << cmd->getName() << " command");
      return false;
    }
    if (args[0].valueType() != STRING_TYPE) {
      warn("First argument to " << cmd->getName() << " command is not a string");
      return false;
    }

    std::string const *nodeName = nullptr;
    if (!args[0].getValuePointer(nodeName)) {
      warn("Node name parameter value to " << cmd->getName() << " command is UNKNOWN");
      return false;
    }

    return true;
  }

  //! Construct the wrapper plan
  static pugi::xml_document makeWrapperPlanXml(const char *callerName,
                                               const char *calleeName,
                                               std::vector<std::string> const &formals,
                                               std::vector<Value> const &actuals)
  {
    pugi::xml_document doc;
    pugi::xml_node plan = doc.append_child("PlexilPlan");
    pugi::xml_node rootNode = plan.append_child("Node");
    rootNode.append_attribute("NodeType").set_value("LibraryNodeCall");
    rootNode.append_child("NodeId").append_child(pugi::node_pcdata)
      .set_value(callerName);

    // Construct ExitCondition
    pugi::xml_node exitLookup = rootNode.append_child("ExitCondition")
      .append_child("LookupNow");
    exitLookup.append_child("Name").append_child("StringValue")
      .append_child(pugi::node_pcdata).set_value(EXIT_PLAN_CMD);
    exitLookup.append_child("Arguments").append_child("StringValue")
      .append_child(pugi::node_pcdata).set_value(callerName);

    pugi::xml_node call = rootNode.append_child("NodeBody")
      .append_child("LibraryNodeCall");
    call.append_child("NodeId").append_child(pugi::node_pcdata).set_value(calleeName);
    for (size_t i = 0; i < formals.size(); ++i) {
      pugi::xml_node alias = call.append_child("Alias");
      alias.append_child("NodeParameter").append_child(pugi::node_pcdata)
        .set_value(formals[i].c_str());
      valueToExprXml(alias, actuals[i]);
    }
    return doc;
  }

  //
  // StartPlan command handler function
  //

  static void executeStartPlanCommand(Command *cmd, AdapterExecInterface *intf)
  {
    if (!checkPlanNameArgument(cmd)) {
      intf->handleCommandAck(cmd, COMMAND_FAILED);
      intf->notifyOfExternalEvent();
      return;
    }

    std::vector<Value> const &args = cmd->getArgValues();
    size_t nargs = args.size();

    std::vector<std::string> formals;
    std::vector<Value> actuals;

    for (size_t i = 1; i < nargs; i += 2) {
      if (i + 1 >= nargs) {
        warn("Launcher: Arguments to " << cmd->getName() << " command not in name-value pairs");
        intf->handleCommandAck(cmd, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }

      if (args[i].valueType() != STRING_TYPE) {
        warn("Launcher: StartPlan command argument " << i << " is not a String");
        intf->handleCommandAck(cmd, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }
      std::string const *formal = nullptr;
      if (!args[i].getValuePointer(formal)) {
        warn("Launcher: StartPlan command argument " << i << " is UNKNOWN");
        intf->handleCommandAck(cmd, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }
      formals.push_back(*formal);

      if (!args[i + 1].isKnown()) {
        warn("Launcher: StartPlan command argument " << i + 1 << " is UNKNOWN");
        intf->handleCommandAck(cmd, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }
      actuals.push_back(args[i + 1]);
    }
    
    // Get the callee's name
    std::string const *nodeName = nullptr;
    args[0].getValuePointer(nodeName); // for effect, value is known to be known

    // Create the caller's name
    std::ostringstream s;
    s << *nodeName << '_' << nextSerialNumber();
    std::string const callerId = s.str();

    // Construct XML for wrapper plan (a LibraryNodeCall node)
    pugi::xml_document const doc =
      makeWrapperPlanXml(callerId.c_str(), nodeName->c_str(), formals, actuals);
    pugi::xml_node const plan = doc.document_element();
    
    try {
      intf->handleAddPlan(plan);
      intf->handleCommandReturn(cmd, Value(callerId));
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
      debugMsg("LauncherAdapter:startPlan",
               ' ' << *nodeName << ": successfully added wrapper plan " << callerId)
    }
    catch (ParserException &e) {
      warn("Launching plan " << nodeName << " failed:\n"
           << e.what());
      intf->handleCommandAck(cmd, COMMAND_FAILED);
      intf->notifyOfExternalEvent();
      return;
    }
  }

  // Find a node by its node ID
  static Node *findNode(std::string const &nodeName)
  {
    // Find the named node
    auto pred = [&nodeName](NodePtr const &n) -> bool
                { return n->getNodeId() == nodeName; };
    std::list<NodePtr> const &allNodes = g_exec->getPlans();
    std::list<NodePtr>::const_iterator it =
      std::find_if(allNodes.begin(), allNodes.end(), pred);
    if (it == allNodes.end()) {
      warn("No such node" << nodeName); // FIXME
      return nullptr;
    }
    Node *result = it->get();
    if (allNodes.end() !=
        std::find_if(++it, allNodes.end(), pred)) {
      warn("Multiple nodes named " << nodeName); // FIXME
      return nullptr;
    }
    return result;
  }

  //! ExitPlan command handler function
  static void executeExitPlanCommand(Command *cmd, AdapterExecInterface *intf)
  {
    if (!checkPlanNameArgument(cmd)) {
      intf->handleCommandAck(cmd, COMMAND_FAILED);
      intf->notifyOfExternalEvent();
      return;
    }

    std::vector<Value> const &args = cmd->getArgValues();
    if (args.size() > 1) {
      warn("Too many parameters to " << cmd->getName() << " command");
      intf->handleCommandAck(cmd, COMMAND_FAILED);
      intf->notifyOfExternalEvent();
      return;
    }

    std::string const *nodeName = nullptr;
    args[0].getValuePointer(nodeName); // for effect, value is known
    Node *node = findNode(*nodeName);
    if (!node) {
      // Not found or multiples with same name
      intf->handleCommandAck(cmd, COMMAND_FAILED);
      intf->notifyOfExternalEvent();
      return;
    }
    intf->handleValueChange(State(EXIT_PLAN_CMD, args[0]), Value(true));
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
    intf->notifyOfExternalEvent();
    debugMsg("LauncherAdapter:exitPlan",
              " exit request sent to " << *nodeName);
  }

  class Launcher : public InterfaceAdapter
  {
  public:
    Launcher(AdapterExecInterface &execInterface, 
             AdapterConf *conf)
      : InterfaceAdapter(execInterface, conf)
    {
    }

    virtual ~Launcher() = default;

    virtual bool initialize(AdapterConfiguration *config) override
    {
      // Register command implementations
      config->registerCommandHandlerFunction(START_PLAN_CMD,
                                             executeStartPlanCommand);
      config->registerCommandHandlerFunction(EXIT_PLAN_CMD,
                                             executeExitPlanCommand);
      debugMsg("LauncherAdapter:initialize", " registered handlers");

      // Register our special ExecListener
      config->addExecListener(new LauncherListener(&this->getInterface()));
      debugMsg("LauncherAdapter:initialize", " registered listener");

      return true;
    }
  };
  
}

extern "C"
void initLauncher()
{
  REGISTER_ADAPTER(PLEXIL::Launcher, "Launcher");
}
