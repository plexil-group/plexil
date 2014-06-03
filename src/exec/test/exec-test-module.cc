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

#include "exec-test-module.hh"

#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExecDefs.hh"
#include "Expression.hh"
#include "ExpressionFactory.hh"
#include "Expressions.hh"
#include "ExternalInterface.hh"
#include "Lookup.hh"
#include "Node.hh"
#include "NodeFactory.hh"
#include "PlexilExec.hh"
#include "PlexilPlan.hh"
#include "TestSupport.hh"
#include "XMLUtils.hh"
#include "lifecycle-utils.h"

#include <iostream>
#include <map>
#include <cmath>

using namespace PLEXIL;

extern bool lookupsTest();

// For Boolean variable/condition tests
#define IDX_UNKNOWN 0
#define IDX_FALSE 1
#define IDX_TRUE 2

class TransitionExecConnector : public ExecConnector {
public:
  TransitionExecConnector() : ExecConnector(), m_executed(false) {}
  void notifyNodeConditionChanged(NodeId /* node */) {}
  void handleConditionsChanged(const NodeId& /* node */, NodeState /* newState */) {}
  void enqueueAssignment(const AssignmentId& /* assign */) {}
  void enqueueAssignmentForRetraction(const AssignmentId& /* assign */) {}
  void enqueueCommand(const CommandId& /* cmd */) {}
  void enqueueUpdate(const UpdateId& /* update */) {}
  // Replacement for handleNeedsExecution()
  void notifyExecuted(const NodeId& node) {assertTrue(node->getState() == EXECUTING_STATE); m_executed = true;}
  void markRootNodeFinished(const NodeId& /* node */) {}
  const StateCacheId& getStateCache() {return StateCacheId::noId();}
  const ExternalInterfaceId& getExternalInterface() {return ExternalInterfaceId::noId();}
  const ExecListenerHubId& getExecListenerHub() const { return ExecListenerHubId::noId(); }
  bool executed() {return m_executed;}
private:
  bool m_executed;
};

class StateTransitionsTest 
{
public:
  static bool test() 
  {
    runTest(inactiveDestTest);
    runTest(inactiveTransTest);
    runTest(waitingDestTest);
    runTest(waitingTransTest);
    runTest(iterationEndedDestTest);
    runTest(iterationEndedTransTest);
    runTest(finishedDestTest);
    runTest(finishedTransTest);
    runTest(listExecutingDestTest);
    runTest(listExecutingTransTest);
    runTest(listFailingDestTest);
    runTest(listFailingTransTest);
    runTest(listFinishingDestTest);
    runTest(listFinishingTransTest);
    runTest(bindingExecutingDestTest);
    runTest(bindingExecutingTransTest);
    runTest(bindingFailingDestTest);
    runTest(bindingFailingTransTest);
    runTest(commandExecutingDestTest);
    runTest(commandExecutingTransTest);
    runTest(commandFailingDestTest);
    runTest(commandFailingTransTest);
    runTest(commandFinishingDestTest);
    runTest(commandFinishingTransTest);
    runTest(updateExecutingDestTest);
    runTest(updateExecutingTransTest);
    runTest(updateFailingDestTest);
    runTest(updateFailingTransTest);
    return true;
  }

private:
  
  static bool inactiveDestTest() 
  {
    TransitionExecConnector con;
    LabelStr types[5] = {Node::ASSIGNMENT(),
                         Node::COMMAND(),
                         Node::LIBRARYNODECALL(),
                         Node::LIST(),
                         Node::UPDATE()};
    NodeState states[7] = {INACTIVE_STATE,
                           WAITING_STATE,
                           EXECUTING_STATE,
                           FINISHING_STATE,
                           FINISHED_STATE,
                           FAILING_STATE,
                           ITERATION_ENDED_STATE};
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (size_t s = 0; s < 7; ++s) {
      NodeId parent =
        NodeFactory::createNode(Node::LIST(),
                                LabelStr("testParent"),
                                states[s],
                                con.getId(),
                                NodeId::noId());
      for (int i = 0; i < 4; ++i) {
        NodeId node = NodeFactory::createNode(types[i],
                                              LabelStr("inactiveDestTest"),
                                              INACTIVE_STATE,
                                              con.getId(), 
                                              parent);

        for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
          node->getAncestorExitCondition()->setValue(values[ancestorExit]);
          for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
            node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
            for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
              node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
              NodeState destState = node->getDestState();
              if (destState != node->getState()) {
                debugMsg("UnitTest:inactiveDestTest",
                         " Node " << node->getNodeId().toString()
                         << " Parent state: " << StateVariable::nodeStateName(states[s])
                         << " Dest: " << StateVariable::nodeStateName(destState));
                if (states[s] == FINISHED_STATE) {
                  assertTrue(destState == FINISHED_STATE);
                }
                else if (states[s] == EXECUTING_STATE) {
                  if (ancestorExit == IDX_TRUE
                      || ancestorInvariant == IDX_FALSE
                      || ancestorEnd == IDX_TRUE) {
                    assertTrue(destState == FINISHED_STATE);
                  }
                  else {
                    assertTrue(destState == WAITING_STATE);
                  }
                }
              }
              else {
                assertTrue(destState == NO_NODE_STATE);
              }
            }
          }
        }
        delete (Node*) node;
      }
      delete (Node*) parent;
    }
    return true;
  }

  static bool inactiveTransTest() 
  {
    TransitionExecConnector con;
    LabelStr types[5] = {Node::ASSIGNMENT(),
                         Node::COMMAND(),
                         Node::LIBRARYNODECALL(),
                         Node::LIST(),
                         Node::UPDATE()};
    NodeState states[7] = {INACTIVE_STATE,
                           WAITING_STATE,
                           EXECUTING_STATE,
                           FINISHING_STATE,
                           FINISHED_STATE,
                           FAILING_STATE,
                           ITERATION_ENDED_STATE};
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    
    for (size_t s = 0; s < 7; ++s) {
      NodeId parent =
        NodeFactory::createNode(Node::LIST(), 
                                LabelStr("testParent"), 
                                states[s],
                                con.getId(), 
                                NodeId::noId());
      for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
            for (int i = 0; i < 5; i++) {
              NodeId node = NodeFactory::createNode(types[i],
                                                    LabelStr("test"), 
                                                    INACTIVE_STATE, 
                                                    con.getId(), 
                                                    parent);
              node->getAncestorExitCondition()->setValue(values[ancestorExit]);
              node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
              node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
      
              if (node->canTransition()) {
                node->transition(node->getDestState());
                NodeState state = node->getState();
                debugMsg("UnitTest:inactiveTransTest",
                         " Node type " << types[i].toString()
                         << " Parent state " << StateVariable::nodeStateName(states[s])
                         << " Node state " << StateVariable::nodeStateName(state));
                if (states[s] == FINISHED_STATE) {
                  assertTrue(state == FINISHED_STATE);
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SKIPPED());
                }
                else if (states[s] == EXECUTING_STATE) {
                  if (ancestorExit == IDX_TRUE
                      || ancestorInvariant == IDX_FALSE
                      || ancestorEnd == IDX_TRUE) {
                    assertTrue(state == FINISHED_STATE);
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SKIPPED());
                  }
                  else {
                    assertTrue(state == WAITING_STATE);
                    // These are activated by the parent node on transition to EXECUTING.
                    //assertTrue(node->isAncestorEndConditionActive());
                    //assertTrue(node->isAncestorExitConditionActive());
                    //assertTrue(node->isAncestorInvariantConditionActive());
                    assertTrue(node->isExitConditionActive());
                    assertTrue(node->isPreConditionActive());
                    assertTrue(node->isSkipConditionActive());
                    assertTrue(node->isStartConditionActive());
                  }
                }
                else {
                  assertTrue(false);
                }
              }
              delete (Node*) node;
            }
          }
        }
      }
      delete (Node*) parent;
    }
    return true;
  }

  static bool waitingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), WAITING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
            node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
            for (int skip = 0; skip < 3; ++skip) {
              node->getSkipCondition()->setValue(values[skip]);
              for (int start = 0; start < 3; ++start) {
                node->getStartCondition()->setValue(values[start]);
                for (int pre = 0; pre < 3; ++pre) {
                  node->getPreCondition()->setValue(values[pre]);

                  NodeState destState = node->getDestState();
                  debugMsg("UnitTest:waitingDestTest: Destination",
                           " state is " << StateVariable::nodeStateName(destState));
                  if (ancestorExit == IDX_TRUE
                      || exit == IDX_TRUE
                      || ancestorInvariant == IDX_FALSE
                      || ancestorEnd == IDX_TRUE
                      || skip == IDX_TRUE) {
                    assertTrue(destState == FINISHED_STATE);
                  }
                  else if (start == IDX_TRUE) {
                    if (pre == IDX_TRUE) {
                      assertTrue(destState == EXECUTING_STATE);
                    }
                    else {
                      assertTrue(destState == ITERATION_ENDED_STATE);
                    }
                  }
                  else {
                    assertTrue(destState == NO_NODE_STATE);
                  }
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool waitingTransTest() 
  {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    LabelStr types[4] = {Node::ASSIGNMENT(), Node::COMMAND(), Node::LIST(), Node::UPDATE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
            for (int skip = 0; skip < 3; ++skip) {
              for (int start = 0; start < 3; ++start) {
                for (int pre = 0; pre < 3; ++pre) {
                  for (int i = 0; i < 4; i++) {
                    NodeId node = NodeFactory::createNode(types[i], LabelStr("test"), WAITING_STATE, con.getId(), parent);
                    node->getAncestorExitCondition()->setValue(values[ancestorExit]);
                    node->getExitCondition()->setValue(values[exit]);
                    node->getSkipCondition()->setValue(values[skip]);
                    node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
                    node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
                    node->getStartCondition()->setValue(values[start]);
                    node->getPreCondition()->setValue(values[pre]);

                    debugMsg("UnitTest:waitingTransition",
                             "Testing node type " << types[i].toString() << " with "
                             << " ancestor exit = " << values[ancestorExit]
                             << " ancestor invariant = " << values[ancestorInvariant]
                             << " ancestor end = " << values[ancestorEnd]
                             << " skip = " << values[skip]
                             << " start = " << values[start]
                             << " pre = " << values[pre]
                             );

                    if (node->canTransition()) {
                      node->transition(node->getDestState());
                      NodeState state = node->getState();

                      if (ancestorExit == IDX_TRUE
                          || exit == IDX_TRUE
                          || ancestorInvariant == IDX_FALSE
                          || ancestorEnd == IDX_TRUE
                          || skip == IDX_TRUE) {
                        assertTrue(state == FINISHED_STATE);
                        assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SKIPPED());
                      }
                      else if (start == IDX_TRUE) {
                        assertTrue(node->isAncestorExitConditionActive());
                        assertTrue(node->isAncestorInvariantConditionActive());
                        if (pre == IDX_TRUE) {
                          assertTrue(state == EXECUTING_STATE);
                          assertTrue(node->isEndConditionActive());
                          assertTrue(node->isExitConditionActive());
                          assertTrue(node->isInvariantConditionActive());
                          if (types[i] == Node::ASSIGNMENT()) {
                            assertTrue(node->isActionCompleteConditionActive());
                          }
                          assertTrue(con.executed());
                        }
                        else {
                          assertTrue(state == ITERATION_ENDED_STATE);
                          assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                          assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PRE_CONDITION_FAILED());
                          assertTrue(node->isRepeatConditionActive());
                          assertTrue(node->isAncestorEndConditionActive());
                        }
                      }
                      else {
                        assertTrue(false);
                      }
                    }
                    delete (Node*) node;
                  }
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool iterationEndedDestTest() {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), ITERATION_ENDED_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
        node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
        for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
          node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
          for (int repeat = 0; repeat < 3; ++repeat) {
            node->getRepeatCondition()->setValue(values[repeat]);

            NodeState destState = node->getDestState();

            if (ancestorExit == IDX_TRUE
                || ancestorInvariant == IDX_FALSE
                || ancestorEnd == IDX_TRUE
                || repeat == IDX_FALSE) {
              assertTrue(destState == FINISHED_STATE);
            }
            else if (repeat == IDX_TRUE) {
              assertTrue(destState == WAITING_STATE);
            }
            else {
              assertTrue(destState == NO_NODE_STATE);
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool iterationEndedTransTest() 
  {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());

    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    LabelStr types[4] = {Node::ASSIGNMENT(), Node::COMMAND(), Node::LIST(), Node::UPDATE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
        for (int ancestorEnd = 0; ancestorEnd < 3; ++ancestorEnd) {
          for (int repeat = 0; repeat < 3; ++repeat) {
            for (int i = 0; i < 4; i++) {
              NodeId node = NodeFactory::createNode(types[i], LabelStr("test"), ITERATION_ENDED_STATE, con.getId(), parent);
              node->getAncestorExitCondition()->setValue(values[ancestorExit]);
              node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
              node->getAncestorEndCondition()->setValue(values[ancestorEnd]);
              node->getRepeatCondition()->setValue(values[repeat]);

              debugMsg("UnitTest:iterationEndedTransition",
                       "Testing node type " << types[i].toString() << " with "
                       << " ancestor exit = " << values[ancestorExit]
                       << " ancestor invariant = " << values[ancestorInvariant]
                       << " ancestor end = " << values[ancestorEnd]
                       << " repeat = " << values[repeat]
                       );

              if (node->canTransition()) {
                node->transition(node->getDestState());
                NodeState state = node->getState();

                // TODO: check to make sure the reset happened here
                if (ancestorExit == IDX_TRUE) {
                  assertTrue(state == FINISHED_STATE);
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                  assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                }
                else if (ancestorInvariant == IDX_FALSE || ancestorEnd == IDX_TRUE || repeat == IDX_FALSE) {
                  assertTrue(state == FINISHED_STATE);
                  if (ancestorInvariant == IDX_FALSE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                  }
                }
                else if (repeat == IDX_TRUE) {
                  assertTrue(state == WAITING_STATE);
                  assertTrue(node->getStartCondition()->isActive());
                }
                else {
                  assertTrue(false);
                }
              }
              delete (Node*) node;
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool finishedDestTest() {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), INACTIVE_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), FINISHED_STATE, con.getId(), parent);
    NodeState states[7] = {INACTIVE_STATE,
                           WAITING_STATE,
                           EXECUTING_STATE,
                           FINISHING_STATE,
                           FINISHED_STATE,
                           FAILING_STATE,
                           ITERATION_ENDED_STATE};

    for (size_t s = 0; s < 7; ++s) {
      parent->setState(states[s]);
      NodeState destState = node->getDestState();
      if (states[s] == WAITING_STATE) {
        assertTrue(destState == INACTIVE_STATE);
      }
      else {
        assertTrue(destState == NO_NODE_STATE);
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool finishedTransTest() {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), INACTIVE_STATE, con.getId(), NodeId::noId());

    NodeState states[7] = {INACTIVE_STATE,
                           WAITING_STATE,
                           EXECUTING_STATE,
                           FINISHING_STATE,
                           FINISHED_STATE,
                           FAILING_STATE,
                           ITERATION_ENDED_STATE};
    LabelStr types[4] = {Node::ASSIGNMENT(), Node::COMMAND(), Node::LIST(), Node::UPDATE()};

    for (size_t s = 0; s < 7; ++s) {
      for (int i = 0; i < 4; ++i) {
        NodeId node = NodeFactory::createNode(types[i], LabelStr("test"), FINISHED_STATE, con.getId(), parent);
        parent->setState(states[s]);

        debugMsg("UnitTest:finishedTransition",
                 "Testing node type " << types[i].toString()
                 << " with parent state = " << StateVariable::nodeStateName(states[s]));

        if (node->canTransition()) {
          node->transition(node->getDestState());
          NodeState state = node->getState();

          if (states[s] == WAITING_STATE) {
            assertTrue(state == INACTIVE_STATE);
          }
          else {
            assertTrue(false);
          }
        }
        delete (Node*) node;
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool listExecutingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int invariant = 0; invariant < 3; ++invariant) {
            node->getInvariantCondition()->setValue(values[invariant]);
            for (int end = 0; end < 3; ++end) {
              node->getEndCondition()->setValue(values[end]);

              NodeState destState = node->getDestState();

              if (ancestorExit == IDX_TRUE
                  || exit == IDX_TRUE
                  || ancestorInvariant == IDX_FALSE 
                  || invariant == IDX_FALSE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (end == IDX_TRUE) {
                assertTrue(destState == FINISHING_STATE);
              }
              else {
                assertTrue(destState == NO_NODE_STATE);
              }
            }
          }
        }
      }
    }

    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool listExecutingTransTest() {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int invariant = 0; invariant < 3; ++invariant) {
            for (int end = 0; end < 3; ++end) {
              NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
              node->getAncestorExitCondition()->setValue(values[ancestorExit]);
              node->getExitCondition()->setValue(values[exit]);
              node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
              node->getInvariantCondition()->setValue(values[invariant]);
              node->getEndCondition()->setValue(values[end]);

              debugMsg("UnitTest:listExecutingTransition",
                       "Testing ListNode with"
                       << " ancestor exit = " << values[ancestorExit]
                       << " exit = " << values[exit]
                       << " ancestor invariant = " << values[ancestorInvariant]
                       << " invariant = " << values[invariant]
                       << " end = " << values[end]);

              if (node->canTransition()) {
                node->transition(node->getDestState());
                NodeState state = node->getState();
                assertTrue(node->getActionCompleteCondition()->isActive());
                if (ancestorExit == IDX_TRUE || exit == IDX_TRUE) {
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                  if (ancestorExit == IDX_TRUE) {
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                  }
                  else if (exit == IDX_TRUE) {
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                  }
                }
                else if (ancestorInvariant == IDX_FALSE || invariant == IDX_FALSE) {
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                  if (ancestorInvariant == IDX_FALSE) {
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                  }
                  else if (invariant == IDX_FALSE) {
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                  }
                }
                else if (end == IDX_TRUE) {
                  assertTrue(state == FINISHING_STATE);
                  assertTrue(node->getInvariantCondition()->isActive());
                  assertTrue(node->isAncestorInvariantConditionActive());
                }
                else {
                  assertTrue(false);
                }
              }
              delete (Node*) node;
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool listFailingDestTest() {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    Value failureTypes[2] = {FailureVariable::PRE_CONDITION_FAILED(), FailureVariable::PARENT_FAILED()};

    for (int children = 0; children < 3; ++children) {
      node->getActionCompleteCondition()->setValue(values[children]);
      for (int failure = 0; failure < 2; ++failure) {
        node->getFailureTypeVariable()->setValue(failureTypes[failure]);
        NodeState destState = node->getDestState();

        if (children == IDX_TRUE) {
          if (failure == 0) {
            assertTrue(destState == ITERATION_ENDED_STATE);
          }
          else if (failure == 1) {
            assertTrue(destState == FINISHED_STATE);
          }
        }
        else {
          assertTrue(destState == NO_NODE_STATE);
        }
      }
    }

    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool listFailingTransTest() {
    TransitionExecConnector con;
    NodeId parent =
      NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());

    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureType[2] = {FailureVariable::INVARIANT_CONDITION_FAILED(), FailureVariable::PARENT_FAILED()};

    for (int children = 0; children < 3; ++children) {
      for (int i = 0; i < 2; ++i) {
        NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
        node->getActionCompleteCondition()->setValue(values[children]);
        node->getFailureTypeVariable()->setValue(failureType[i]);

        debugMsg("UnitTest:listFailingTrans",
                 "Testing with children waiting or finished = " << values[children]
                 << " failure type = " << failureType[i]);

        if (node->canTransition()) {
          node->transition(node->getDestState());
          NodeState state = node->getState();

          if (children == IDX_TRUE) {
            if (i == 0) {
              assertTrue(state == ITERATION_ENDED_STATE);
              assertTrue(node->isAncestorInvariantConditionActive());
              assertTrue(node->isAncestorEndConditionActive());
              assertTrue(node->isRepeatConditionActive());
            }
            else if (i == 1) {
              assertTrue(state == FINISHED_STATE);
            }
            else {
              assertTrue(false);
            }
          }
          else {
            assertTrue(false);
          }
        }
        delete (Node*) node;
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool listFinishingDestTest()
 {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), FINISHING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int invariant = 0; invariant < 3; ++invariant) {
            node->getInvariantCondition()->setValue(values[invariant]);
            for (int children = 0; children < 3; ++children) {
              node->getActionCompleteCondition()->setValue(values[children]);
              for (int post = 0; post < 3; ++post) {
                node->getPostCondition()->setValue(values[post]);
                NodeState destState = node->getDestState();

                debugMsg("UnitTest:listFinishingDest",
                         "Testing NodeList with"
                         << " ancestor exit = " << values[ancestorExit]
                         << " exit = " << values[exit]
                         << " ancestor invariant = " << values[ancestorInvariant]
                         << " invariant = " << values[invariant]
                         << " children waiting or finished = " << values[children]
                         << " post = " << values[post]
                         << "\n Got dest " << StateVariable::nodeStateName(destState));

                if (ancestorExit == IDX_TRUE
                    || exit == IDX_TRUE
                    || ancestorInvariant == IDX_FALSE
                    || invariant == IDX_FALSE) {
                  assertTrue(destState == FAILING_STATE);
                }
                else if (children == IDX_TRUE) {
                  assertTrue(destState == ITERATION_ENDED_STATE);
                }
                else {
                  assertTrue(destState == NO_NODE_STATE);
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool listFinishingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());

    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int invariant = 0; invariant < 3; ++invariant) {
            for (int children = 0; children < 3; ++children) {
              for (int post = 0; post < 3; ++post) {
                NodeId node = NodeFactory::createNode(Node::LIST(), LabelStr("test"), FINISHING_STATE, con.getId(), parent);
                node->getAncestorExitCondition()->setValue(values[ancestorExit]);
                node->getExitCondition()->setValue(values[exit]);
                node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
                node->getInvariantCondition()->setValue(values[invariant]);
                node->getActionCompleteCondition()->setValue(values[children]);
                node->getPostCondition()->setValue(values[post]);

                debugMsg("UnitTest:listFinishingTrans",
                         "Testing NodeList with"
                         << " ancestor exit = " << values[ancestorExit]
                         << " exit = " << values[exit]
                         << " ancestor invariant = " << values[ancestorInvariant]
                         << " invariant = " << values[invariant]
                         << " children waiting or finished = " << values[children]
                         << " post = " << values[post]);

                if (node->canTransition()) {
                  node->transition(node->getDestState());
                  NodeState state = node->getState();

                  if (ancestorExit == IDX_TRUE || exit == IDX_TRUE) {
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                    if (ancestorExit == IDX_TRUE) {
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                    }
                    else if (exit == IDX_TRUE) {
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                    }
                    assertTrue(node->getActionCompleteCondition()->isActive());
                  }
                  else if (ancestorInvariant == IDX_FALSE || invariant == IDX_FALSE) {
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    if (ancestorInvariant == IDX_FALSE) {
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                    }
                    else if (invariant == IDX_FALSE) {
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                    }
                    assertTrue(node->getActionCompleteCondition()->isActive());
                  }
                  else if (children == IDX_TRUE) {
                    assertTrue(state == ITERATION_ENDED_STATE);
                    assertTrue(node->isAncestorInvariantConditionActive());
                    assertTrue(node->isAncestorExitConditionActive());
                    assertTrue(node->isAncestorEndConditionActive());
                    assertTrue(node->isRepeatConditionActive());
                    if (post == IDX_TRUE) {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SUCCESS());
                    }
                    else {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::POST_CONDITION_FAILED());
                    }
                  }
                  else {
                    assertTrue(false);
                  }
                }
                delete (Node*) node;
              }
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool bindingExecutingDestTest() 
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
      node->getActionCompleteCondition()->setValue(values[actionComplete]);
      for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
        node->getAncestorExitCondition()->setValue(values[ancestorExit]);
        for (int exit = 0; exit < 3; ++exit) {
          node->getExitCondition()->setValue(values[exit]);
          for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
            node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
            for (int invariant = 0; invariant < 3; ++invariant) {
              node->getInvariantCondition()->setValue(values[invariant]);
              for (int end = 0; end < 3; ++end) {
                node->getEndCondition()->setValue(values[end]);
                for (int post = 0; post < 3; ++post) {
                  node->getPostCondition()->setValue(values[post]);

                  NodeState destState = node->getDestState();
                  if (actionComplete != IDX_TRUE) {
                    assertTrue(destState == NO_NODE_STATE);
                  }
                  else if (ancestorExit == IDX_TRUE) {
                    assertTrue(destState == FAILING_STATE);
                  }
                  else if (exit == IDX_TRUE) {
                    assertTrue(destState == FAILING_STATE);
                  }
                  else if (ancestorInvariant == IDX_FALSE) {
                    assertTrue(destState == FAILING_STATE);
                  }
                  else if (invariant == IDX_FALSE) {
                    assertTrue(destState == FAILING_STATE);
                  }
                  else if (end == IDX_TRUE) {
                    assertTrue(destState == ITERATION_ENDED_STATE);
                  }
                  else {
                    assertTrue(destState == NO_NODE_STATE);
                  }
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool bindingExecutingTransTest() 
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());

    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    LabelStr type = Node::ASSIGNMENT();

    for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
      for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
        for (int exit = 0; exit < 3; ++exit) {
          for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
            for (int invariant = 0; invariant < 3; ++invariant) {
              for (int end = 0; end < 3; ++end) {
                for (int post = 0; end < 3; ++end) {
                  NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
                  node->getActionCompleteCondition()->setValue(values[actionComplete]);
                  node->getAncestorExitCondition()->setValue(values[ancestorExit]);
                  node->getExitCondition()->setValue(values[exit]);
                  node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
                  node->getInvariantCondition()->setValue(values[invariant]);
                  node->getEndCondition()->setValue(values[end]);
                  node->getPostCondition()->setValue(values[post]);

                  debugMsg("UnitTest:bindingExecutingTransition",
                           "Testing " << Node::ASSIGNMENT().toString() << " with"
                           << " action complete = " << values[actionComplete]
                           << " ancestor exit = " << values[ancestorExit]
                           << " exit = " << values[exit]
                           << " ancestor invariant = " << values[ancestorInvariant]
                           << " invariant = " << values[invariant]
                           << " end = " << values[end]
                           << " post = " << values[post]
                           );

                  if (node->canTransition()) {
                    node->transition(node->getDestState());
                    NodeState state = node->getState();

                    if (ancestorExit == IDX_TRUE || exit == IDX_TRUE) {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                      if (ancestorExit == IDX_TRUE) {
                        assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                        assertTrue(state == FAILING_STATE);
                        assertTrue(node->isAbortCompleteConditionActive());
                      }
                      else if (exit == IDX_TRUE) {
                        assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                        assertTrue(state == FAILING_STATE);
                        assertTrue(node->isAbortCompleteConditionActive());
                      }
                    }
                    else if (ancestorInvariant == IDX_FALSE || invariant == IDX_FALSE) {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                      if (ancestorInvariant == IDX_FALSE) {
                        assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                        assertTrue(state == FAILING_STATE);
                        assertTrue(node->isAbortCompleteConditionActive());
                      }
                      else if (invariant == IDX_FALSE) {
                        assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                        assertTrue(state == FAILING_STATE);
                        assertTrue(node->isAbortCompleteConditionActive());
                      }
                    }
                    else if (end == IDX_TRUE) {
                      assertTrue(state == ITERATION_ENDED_STATE);
                      assertTrue(node->isAncestorEndConditionActive());
                      assertTrue(node->isAncestorExitConditionActive());
                      assertTrue(node->isAncestorInvariantConditionActive());
                      assertTrue(node->isRepeatConditionActive());
                      if (post == IDX_TRUE) {
                        assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SUCCESS());
                      }
                      else {
                        assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                        assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::POST_CONDITION_FAILED());
                      }
                    }
                    else {
                      assertTrue(false);
                    }
                  }
                  delete (Node*) node;
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool bindingFailingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureType[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                            FailureVariable::PARENT_FAILED(),
                            FailureVariable::EXITED(),
                            FailureVariable::PARENT_EXITED()};

    for (int abortComplete = 0; abortComplete < 3; ++abortComplete) {
      node->getAbortCompleteCondition()->setValue(values[abortComplete]);
      for (int failure = 0; failure < 4; ++ failure) {
        node->getFailureTypeVariable()->setValue(failureType[failure]);

        NodeState destState = node->getDestState();
        if (abortComplete == IDX_TRUE) {
          if (failureType[failure] == FailureVariable::PARENT_FAILED()
              || failureType[failure] == FailureVariable::PARENT_EXITED()) {
            assertTrue(destState == FINISHED_STATE);
          }
          else {
            assertTrue(destState == ITERATION_ENDED_STATE);
          }
        }
        else {
          assertTrue(destState == NO_NODE_STATE);
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool bindingFailingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureType[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                            FailureVariable::PARENT_FAILED(),
                            FailureVariable::EXITED(),
                            FailureVariable::PARENT_EXITED()};

    for (int children = 0; children < 3; ++children) {
      for (int failure = 0; failure < 2; ++failure) {
        NodeId node = NodeFactory::createNode(Node::ASSIGNMENT(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
        node->getActionCompleteCondition()->setValue(values[children]);
        node->getFailureTypeVariable()->setValue(failureType[failure]);

        debugMsg("UnitTest:listFailingTrans",
                 "Testing with children waiting or finished = " << values[children]
                 << " failure type = " << failureType[failure]);

        if (node->canTransition()) {
          node->transition(node->getDestState());
          NodeState state = node->getState();

          if (children == IDX_TRUE) {
            if (failureType[failure] == FailureVariable::PARENT_FAILED()
                || failureType[failure] == FailureVariable::PARENT_EXITED()) {
              assertTrue(state == FINISHED_STATE);
            }
            else {
              assertTrue(state == ITERATION_ENDED_STATE);
              assertTrue(node->isAncestorInvariantConditionActive());
              assertTrue(node->isAncestorEndConditionActive());
              assertTrue(node->isRepeatConditionActive());
            }
          }
          else {
            assertTrue(false);
          }
        }
        delete (Node*) node;
      }
    }
    delete (Node*) parent;
    return true;
  }

  //
  // Command nodes
  // 

  static bool commandExecutingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int invariant = 0; invariant < 3; ++invariant) {
            node->getInvariantCondition()->setValue(values[invariant]);
            for (int end = 0; end < 3; ++end) {
              node->getEndCondition()->setValue(values[end]);

              NodeState destState = node->getDestState();
              if (ancestorExit == IDX_TRUE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (exit == IDX_TRUE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (ancestorInvariant == IDX_FALSE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (invariant == IDX_FALSE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (end == IDX_TRUE) {
                assertTrue(destState == FINISHING_STATE);
              }
              else {
                assertTrue(destState == NO_NODE_STATE);
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool commandExecutingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());

    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int invariant = 0; invariant < 3; ++invariant) {
            for (int end = 0; end < 3; ++end) {
              NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
              node->getAncestorExitCondition()->setValue(values[ancestorExit]);
              node->getExitCondition()->setValue(values[exit]);
              node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
              node->getInvariantCondition()->setValue(values[invariant]);
              node->getEndCondition()->setValue(values[end]);
              debugMsg("UnitTest:actionExecutingTransition",
                       "Testing Command with"
                       << " ancestor exit = " << values[ancestorExit]
                       << " exit = " << values[exit]
                       << " ancestor invariant = " << values[ancestorInvariant]
                       << " invariant = " << values[invariant]
                       << " end = " << values[end]
                       );

              if (node->canTransition()) {
                node->transition(node->getDestState());
                NodeState state = node->getState();
                if (ancestorExit == IDX_TRUE) {
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                  assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->isAbortCompleteConditionActive());
                }
                else if (exit == IDX_TRUE) {
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                  assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->isAbortCompleteConditionActive());
                }
                else if (ancestorInvariant == IDX_FALSE) {
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                  assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->isAbortCompleteConditionActive());
                }
                else if (invariant == IDX_FALSE) {
                  assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                  assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                  assertTrue(state == FAILING_STATE);
                  assertTrue(node->isAbortCompleteConditionActive());
                }
                else if (end == IDX_TRUE) {
                  assertTrue(state == FINISHING_STATE);
                  assertTrue(node->isActionCompleteConditionActive());
                  assertTrue(node->isAncestorExitConditionActive());
                  assertTrue(node->isAncestorInvariantConditionActive());
                  assertTrue(node->isExitConditionActive());
                  assertTrue(node->isInvariantConditionActive());
                  assertTrue(node->isPostConditionActive());
                }
                else {
                  assertTrue(false);
                }
              }
              delete (Node*) node;
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool commandFailingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureTypes[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                             FailureVariable::PARENT_FAILED(),
                             FailureVariable::EXITED(),
                             FailureVariable::PARENT_EXITED()};
    for (int abortComplete = 0; abortComplete < 3; ++abortComplete) {
      node->getAbortCompleteCondition()->setValue(values[abortComplete]);
      for (int failure = 0; failure < 4; ++failure) {
        node->getFailureTypeVariable()->setValue(failureTypes[failure]);
        NodeState destState = node->getDestState();
        if (abortComplete == IDX_TRUE) {
          if (failureTypes[failure] == FailureVariable::PARENT_FAILED()
              || failureTypes[failure] == FailureVariable::PARENT_EXITED()) {
            assertTrue(destState == FINISHED_STATE);
          }
          else {
            assertTrue(destState == ITERATION_ENDED_STATE);
          }
        }
        else {
          assertTrue(destState == NO_NODE_STATE);
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool commandFailingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureTypes[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                             FailureVariable::PARENT_FAILED(),
                             FailureVariable::EXITED(),
                             FailureVariable::PARENT_EXITED()};

    for (int abort = 0; abort < 3; ++abort) {
      for (int failure = 0; failure < 4; ++failure) {
        NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
        node->getAbortCompleteCondition()->setValue(values[abort]);
        node->getFailureTypeVariable()->setValue(failureTypes[failure]);

        debugMsg("UnitTest:finishedTransition",
                 "Testing Command with"
                 << " abort complete = " << values[abort]
                 << " failure type = " << failureTypes[failure]);

        if (node->canTransition()) {
          node->transition(node->getDestState());
          NodeState state = node->getState();

          if (abort == IDX_TRUE) {
            if (failureTypes[failure] == FailureVariable::PARENT_FAILED()
                || failureTypes[failure] == FailureVariable::PARENT_EXITED()) {
              assertTrue(state == FINISHED_STATE);
            }
            else {
              assertTrue(state == ITERATION_ENDED_STATE);
              assertTrue(node->isRepeatConditionActive());
              assertTrue(node->isAncestorEndConditionActive());
              assertTrue(node->isAncestorExitConditionActive());
              assertTrue(node->isAncestorInvariantConditionActive());
            }
          }
          else {
            assertTrue(false);
          }
        }
        delete (Node*) node;
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool commandFinishingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), FINISHING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int invariant = 0; invariant < 3; ++invariant) {
            node->getInvariantCondition()->setValue(values[invariant]);
            for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
              node->getActionCompleteCondition()->setValue(values[actionComplete]);

              NodeState destState = node->getDestState();
              if (ancestorExit == IDX_TRUE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (exit == IDX_TRUE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (ancestorInvariant == IDX_FALSE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (invariant == IDX_FALSE) {
                assertTrue(destState == FAILING_STATE);
              }
              else if (actionComplete == IDX_TRUE) {
                assertTrue(destState == ITERATION_ENDED_STATE);
              }
              else {
                assertTrue(destState == NO_NODE_STATE);
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool commandFinishingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int invariant = 0; invariant < 3; ++invariant) {
            for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
              for (int post = 0; post < 3; ++post) {
                NodeId node = NodeFactory::createNode(Node::COMMAND(), LabelStr("test"), FINISHING_STATE, con.getId(), parent);
                node->getAncestorExitCondition()->setValue(values[ancestorExit]);
                node->getExitCondition()->setValue(values[exit]);
                node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
                node->getInvariantCondition()->setValue(values[invariant]);
                node->getActionCompleteCondition()->setValue(values[actionComplete]);
                node->getPostCondition()->setValue(values[post]);
                debugMsg("UnitTest:actionExecutingTransition",
                         "Testing Command with"
                         << " ancestor exit = " << values[ancestorExit]
                         << " exit = " << values[exit]
                         << " ancestor invariant = " << values[ancestorInvariant]
                         << " invariant = " << values[invariant]
                         << " action complete = " << values[actionComplete]
                         << " post = " << values[post]
                         );

                if (node->canTransition()) {
                  node->transition(node->getDestState());
                  NodeState state = node->getState();
                  if (ancestorExit == IDX_TRUE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isAbortCompleteConditionActive());
                  }
                  else if (exit == IDX_TRUE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isAbortCompleteConditionActive());
                  }
                  else if (ancestorInvariant == IDX_FALSE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isAbortCompleteConditionActive());
                  }
                  else if (invariant == IDX_FALSE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isAbortCompleteConditionActive());
                  }
                  else if (actionComplete == IDX_TRUE) {
                    assertTrue(state == ITERATION_ENDED_STATE);
                    assertTrue(node->isAncestorEndConditionActive());
                    assertTrue(node->isAncestorExitConditionActive());
                    assertTrue(node->isAncestorInvariantConditionActive());
                    assertTrue(node->isRepeatConditionActive());
                    if (post == IDX_TRUE) {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SUCCESS());
                    }
                    else {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::POST_CONDITION_FAILED());
                    }
                  }
                  else {
                    assertTrue(false);
                  }
                }
                delete (Node*) node;
              }
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  //
  // Update nodes
  //

  static bool updateExecutingDestTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::UPDATE(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      node->getAncestorExitCondition()->setValue(values[ancestorExit]);
      for (int exit = 0; exit < 3; ++exit) {
        node->getExitCondition()->setValue(values[exit]);
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
          for (int invariant = 0; invariant < 3; ++invariant) {
            node->getInvariantCondition()->setValue(values[invariant]);
            for (int end = 0; end < 3; ++end) {
              node->getEndCondition()->setValue(values[end]);
              for (int post = 0; post < 3; ++post) {
                node->getPostCondition()->setValue(values[post]);

                NodeState destState = node->getDestState();
                if (ancestorExit == IDX_TRUE) {
                  assertTrue(destState == FAILING_STATE);
                }
                else if (exit == IDX_TRUE) {
                  assertTrue(destState == FAILING_STATE);
                }
                else if (ancestorInvariant == IDX_FALSE) {
                  assertTrue(destState == FAILING_STATE);
                }
                else if (invariant == IDX_FALSE) {
                  assertTrue(destState == FAILING_STATE);
                }
                else if (end == IDX_TRUE) {
                  assertTrue(destState == ITERATION_ENDED_STATE);
                }
                else {
                  assertTrue(destState == NO_NODE_STATE);
                }
              }
            }
          }
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool updateExecutingTransTest()
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};

    for (int ancestorExit = 0; ancestorExit < 3; ++ancestorExit) {
      for (int exit = 0; exit < 3; ++exit) {
        for (int ancestorInvariant = 0; ancestorInvariant < 3; ++ancestorInvariant) {
          for (int invariant = 0; invariant < 3; ++invariant) {
            for (int end = 0; end < 3; ++end) {
              for (int post = 0; post < 3; ++post) {
                NodeId node = NodeFactory::createNode(Node::UPDATE(), LabelStr("test"), EXECUTING_STATE, con.getId(), parent);
                node->getAncestorExitCondition()->setValue(values[ancestorExit]);
                node->getExitCondition()->setValue(values[exit]);
                node->getAncestorInvariantCondition()->setValue(values[ancestorInvariant]);
                node->getInvariantCondition()->setValue(values[invariant]);
                node->getEndCondition()->setValue(values[end]);
                node->getPostCondition()->setValue(values[post]);
                debugMsg("UnitTest:actionExecutingTransition",
                         "Testing Update with"
                         << " ancestor exit = " << values[ancestorExit]
                         << " exit = " << values[exit]
                         << " ancestor invariant = " << values[ancestorInvariant]
                         << " invariant = " << values[invariant]
                         << " end = " << values[end]
                         << " post = " << values[post]
                         );

                if (node->canTransition()) {
                  node->transition(node->getDestState());
                  NodeState state = node->getState();
                  if (ancestorExit == IDX_TRUE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_EXITED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isActionCompleteConditionActive());
                  }
                  else if (exit == IDX_TRUE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::INTERRUPTED());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::EXITED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isActionCompleteConditionActive());
                  }
                  else if (ancestorInvariant == IDX_FALSE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isActionCompleteConditionActive());
                  }
                  else if (invariant == IDX_FALSE) {
                    assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                    assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::INVARIANT_CONDITION_FAILED());
                    assertTrue(state == FAILING_STATE);
                    assertTrue(node->isActionCompleteConditionActive());
                  }
                  else if (end == IDX_TRUE) {
                    assertTrue(state == ITERATION_ENDED_STATE);
                    assertTrue(node->isAncestorEndConditionActive());
                    assertTrue(node->isAncestorExitConditionActive());
                    assertTrue(node->isAncestorInvariantConditionActive());
                    if (post == IDX_TRUE) {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::SUCCESS());
                    }
                    else {
                      assertTrue(node->getOutcomeVariable()->getValue() == OutcomeVariable::FAILURE());
                      assertTrue(node->getFailureTypeVariable()->getValue() == FailureVariable::POST_CONDITION_FAILED());
                    }
                  }
                  else {
                    assertTrue(false);
                  }
                }
                delete (Node*) node;
              }
            }
          }
        }
      }
    }
    delete (Node*) parent;
    return true;
  }

  static bool updateFailingDestTest() 
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    NodeId node = NodeFactory::createNode(Node::UPDATE(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureTypes[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                             FailureVariable::PARENT_FAILED(),
                             FailureVariable::EXITED(),
                             FailureVariable::PARENT_EXITED()};
    for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
      node->getActionCompleteCondition()->setValue(values[actionComplete]);
      for (int failure = 0; failure < 4; ++failure) {
        node->getFailureTypeVariable()->setValue(failureTypes[failure]);
        NodeState destState = node->getDestState();
        if (actionComplete == IDX_TRUE) {
          if (failureTypes[failure] == FailureVariable::PARENT_FAILED()
              || failureTypes[failure] == FailureVariable::PARENT_EXITED()) {
            assertTrue(destState == FINISHED_STATE);
          }
          else {
            assertTrue(destState == ITERATION_ENDED_STATE);
          }
        }
        else {
          assertTrue(destState == NO_NODE_STATE);
        }
      }
    }
    delete (Node*) node;
    delete (Node*) parent;
    return true;
  }

  static bool updateFailingTransTest() 
  {
    TransitionExecConnector con;
    NodeId parent = NodeFactory::createNode(Node::LIST(), LabelStr("testParent"), EXECUTING_STATE, con.getId(), NodeId::noId());
    Value values[3] = {UNKNOWN(), BooleanVariable::FALSE_VALUE(), BooleanVariable::TRUE_VALUE()};
    Value failureTypes[4] = {FailureVariable::INVARIANT_CONDITION_FAILED(),
                             FailureVariable::PARENT_FAILED(),
                             FailureVariable::EXITED(),
                             FailureVariable::PARENT_EXITED()};

    for (int actionComplete = 0; actionComplete < 3; ++actionComplete) {
      for (int failure = 0; failure < 4; ++failure) {
        NodeId node = NodeFactory::createNode(Node::UPDATE(), LabelStr("test"), FAILING_STATE, con.getId(), parent);
        node->getActionCompleteCondition()->setValue(values[actionComplete]);
        node->getFailureTypeVariable()->setValue(failureTypes[failure]);

        debugMsg("UnitTest:finishedTransition",
                 "Testing Update with"
                 << " action complete = " << values[actionComplete]
                 << " failure type = " << failureTypes[failure]);

        if (node->canTransition()) {
          node->transition(node->getDestState());
          NodeState state = node->getState();

          if (actionComplete == IDX_TRUE) {
            if (failureTypes[failure] == FailureVariable::PARENT_FAILED()
                || failureTypes[failure] == FailureVariable::PARENT_EXITED()) {
              assertTrue(state == FINISHED_STATE);
            }
            else {
              assertTrue(state == ITERATION_ENDED_STATE);
              assertTrue(node->isRepeatConditionActive());
              assertTrue(node->isAncestorEndConditionActive());
              assertTrue(node->isAncestorExitConditionActive());
              assertTrue(node->isAncestorInvariantConditionActive());
            }
          }
          else {
            assertTrue(false);
          }
        }
        delete (Node*) node;
      }
    }
    delete (Node*) parent;
    return true;
  }

};

class CacheTestInterface : public ExternalInterface {
public:
  CacheTestInterface(const StateCacheId& cache)
  : ExternalInterface(), m_cache(cache), m_lookupNowCalled(false)
  {}

  Value lookupNow(const State& state) 
  {
    m_lookupNowCalled = true;
    return m_values[state];
  }

  void subscribe(const State& /* state */)
  {
  }

  void unsubscribe(const State& /* state */)
  {
  }

  void setThresholds(const State& /* state */, double /* hi */, double /* lo */)
  {
  }

  void batchActions(std::list<CommandId>& /* commands */)
  {
  }

  void updatePlanner(std::list<UpdateId>& /* updates */)
  {
  }

  void invokeAbort(const CommandId& /* cmd */)
  {
  }

  double currentTime()
  {
    return 0.0;
  }

  bool lookupNowCalled() {return m_lookupNowCalled;}
  void clearLookupNowCalled() {m_lookupNowCalled = false;}
  void setValue(const State& state, double value, StateCacheId cache, bool update = true) {
    std::map<State, Value>::iterator it = m_values.find(state);
    if (it == m_values.end())
      m_values.insert(std::make_pair(state, Value(value)));
    else
      it->second = Value(value);
    if (update)
      cache->updateState(state, Value(value));
  }
protected:
private:
  std::map<State, Value> m_values;
  StateCacheId m_cache;
  bool m_lookupNowCalled;
};

class StateCacheTest {
public:
  static bool test() {
    runTest(testLookupNow);
    runTest(testChangeLookup);
    return true;
  }
private:
  static bool testLookupNow() {
    StateCache cache;
    CacheTestInterface iface(cache.getId());
    cache.setExternalInterface(iface.getId());

    IntegerVariable destVar;
    destVar.activate();

    State st("foo", std::vector<Value>());

    iface.setValue(st, 1, cache.getId(), false);
    cache.handleQuiescenceStarted();

    //single lookup for new state
    assertTrue(destVar.getValue().isUnknown());
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 1);
    cache.unregisterLookupNow(destVar.getId());

    //re-lookup for same state in same quiescence
    iface.setValue(st, 2, cache.getId(), false);
    iface.clearLookupNowCalled();
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(!iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 1);
    cache.unregisterLookupNow(destVar.getId());

    //re-lookup for same state in next quiescence
    cache.handleQuiescenceEnded();
    cache.handleQuiescenceStarted();
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 2);
    cache.unregisterLookupNow(destVar.getId());

    // *** TODO: Add test for updating LookupNow that 
    // *** remains active across multiple quiescence cycles

    return true;
  }

  static bool testChangeLookup() {
    StateCache cache;
    CacheTestInterface iface(cache.getId());
    cache.setExternalInterface(iface.getId());

    IntegerVariable destVar1, destVar2;
    destVar1.activate();
    destVar2.activate();

    State st("foo", std::vector<Value>());

    //lookup
    iface.setValue(st, 1, cache.getId(), false);
    cache.handleQuiescenceStarted();
    cache.registerChangeLookup(destVar1.getId(), st, 1);
    assertTrue(destVar1.getValue().getDoubleValue() == 1);
    cache.registerChangeLookup(destVar2.getId(), st, 2);
    assertTrue(destVar2.getValue().getDoubleValue() == 1);
    cache.handleQuiescenceEnded();

    //update value
    iface.setValue(st, 2, cache.getId());
    assertTrue(destVar1.getValue().getDoubleValue() == 2);
    assertTrue(destVar2.getValue().getDoubleValue() == 1);

    //lookupNow triggering change
    IntegerVariable nowDestVar;
    nowDestVar.activate();
    iface.setValue(st, 3, cache.getId(), false);
    cache.handleQuiescenceStarted();
    cache.handleQuiescenceEnded();
    cache.handleQuiescenceStarted();
    cache.registerLookupNow(nowDestVar.getId(), st);
    assertTrue(nowDestVar.getValue().getDoubleValue() == 3);
    assertTrue(destVar1.getValue().getDoubleValue() == 3);
    assertTrue(destVar2.getValue().getDoubleValue() == 3);

    //unregister
    cache.unregisterLookupNow(nowDestVar.getId());
    cache.unregisterChangeLookup(destVar2.getId());
    cache.handleQuiescenceEnded();
    iface.setValue(st, 5, cache.getId());
    assertTrue(destVar2.getValue().getDoubleValue() == 3);
    assertTrue(destVar1.getValue().getDoubleValue() == 5);
    return true;
  }
};


void ExecModuleTests::runTests() {
  REGISTER_EXPRESSION(Conjunction, AND);
  REGISTER_EXPRESSION(Disjunction, OR);
  REGISTER_EXPRESSION(ExclusiveDisjunction, XOR);
  REGISTER_EXPRESSION(LogicalNegation, NOT);
  REGISTER_EXPRESSION(IsKnown, IsKnown);
  REGISTER_EXPRESSION(Equality, EQ);
  REGISTER_EXPRESSION(Equality, EQNumeric);
  REGISTER_EXPRESSION(Equality, EQBoolean);
  REGISTER_EXPRESSION(Equality, EQString);
  REGISTER_EXPRESSION(Inequality, NE);
  REGISTER_EXPRESSION(Inequality, NENumeric);
  REGISTER_EXPRESSION(Inequality, NEBoolean);
  REGISTER_EXPRESSION(Inequality, NEString);
  REGISTER_EXPRESSION(LessThan, LT);
  REGISTER_EXPRESSION(LessEqual, LE);
  REGISTER_EXPRESSION(GreaterThan, GT);
  REGISTER_EXPRESSION(GreaterEqual, GE);
  REGISTER_EXPRESSION(Addition, ADD);
  REGISTER_EXPRESSION(Subtraction, SUB);
  REGISTER_EXPRESSION(Multiplication, MUL);
  REGISTER_EXPRESSION(Division, DIV);
  REGISTER_EXPRESSION(ArrayVariable, Array);
  REGISTER_CONSTANT_EXPRESSION(ArrayVariable, ArrayValue);
  REGISTER_EXPRESSION(BooleanVariable, Boolean);
  REGISTER_CONSTANT_EXPRESSION(BooleanVariable, BooleanValue);
  REGISTER_EXPRESSION(IntegerVariable, Integer);
  REGISTER_CONSTANT_EXPRESSION(IntegerVariable, IntegerValue);
  REGISTER_EXPRESSION(RealVariable, Real);
  REGISTER_CONSTANT_EXPRESSION(RealVariable, RealValue);
  REGISTER_EXPRESSION(StringVariable, String);
  REGISTER_CONSTANT_EXPRESSION(StringVariable, StringValue);
  REGISTER_EXPRESSION(StateVariable, NodeStateValue);
  REGISTER_EXPRESSION(OutcomeVariable, NodeOutcomeValue);
  REGISTER_EXPRESSION(InternalCondition, EQInternal);
  REGISTER_EXPRESSION(InternalCondition, NEInternal);
  REGISTER_EXPRESSION(LookupNow, LookupNow);
  REGISTER_EXPRESSION(LookupOnChange, LookupOnChange);
  REGISTER_EXPRESSION(AbsoluteValue, ABS);
  REGISTER_EXPRESSION(TimepointVariable, NodeTimepointValue);
  //these are to make sure that the id count gets captured properly
  BooleanVariable::FALSE_EXP();
  BooleanVariable::TRUE_EXP();

  runTestSuite(LookupsTest::test);
  runTestSuite(StateTransitionsTest::test);
  runTestSuite(StateCacheTest::test);

  // Clean up
  runFinalizers();

  std::cout << "Finished" << std::endl;
}
