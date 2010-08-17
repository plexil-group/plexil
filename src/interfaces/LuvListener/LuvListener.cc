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

#include "LuvListener.hh"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Socket.h"

#include "Expression.hh"
#include "CoreExpressions.hh"
#include "Expressions.hh"

#include "Node.hh"

#include "PlexilExec.hh"
#include "PlexilPlan.hh"
#include "PlexilXmlParser.hh"
#include "LabelStr.hh"

#include <ios>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <pthread.h>
#include <iomanip>

#define TRUE_STR              "true"
#define FALSE_STR             "false"
#define PLAN_INFO_STR         "PlanInfo"
#define PLEXIL_PLAN_STR       "PlexilPlan"
#define PLEXIL_LIBRARY_STR    "PlexilLibrary"
#define VIEWER_BLOCKS_STR     "ViewerBlocks"

#define NODE_ID_STR           "NodeId"
#define NODE_PATH_STR         "NodePath"

#define NODE_STATE_UPDATE_STR "NodeStateUpdate"
#define NODE_STATE_STR        "NodeState"
#define NODE_OUTCOME_STR      "NodeOutcome"
#define NODE_FAILURE_TYPE_STR "NodeFailureType"
#define CONDITIONS_STR        "Conditions"

namespace PLEXIL 
{
  LuvServer::LuvServer(const int port, const bool block, PlexilExecId exec)  
    : m_port(port), m_block(block), m_exec(exec)
  {
    // create the thread which listens for connections

    if (pthread_create(&m_thread, NULL, handleConnection, (void*)this) != 0)
      throw SocketException("Could not create listen thread.");
  }

  void* handleConnection(void* ptr)
  {
    LuvServer* luvServer = (LuvServer*)ptr;
      
    // run forever
      
    while (true)
      {
        // create a waiting socket
        SHOW("new socket");
        ServerSocket ss(luvServer->getPort());
        SHOW(luvServer->getPort());

        // if we've got a connection

        if (ss.accept(ss))
          {
            SHOW("connection");

            // create a listener and attach it to the exec

            LuvListener* ll = new LuvListener(&ss, luvServer->getBlock());
            luvServer->getExec()->addListener(ll->getId());

            // wait for the connection to go down

            //             while (ss.isOpen())
            //             {
            //                std::cout << "." << std::flush;
            //                usleep(VULTURE_DELAY);
            //             }

            SHOW("loop");
          }
      }

    return ptr;
  }


  // create a listener
  LuvListener::LuvListener()
    : ExecListener(),
      m_socket(NULL),
      m_block(false)
  {
  }

  // create a listener given a host and port

  LuvListener::LuvListener(const std::string& host, 
                           const int port, 
                           const bool block)
    : ExecListener(),
      m_socket(NULL),
      m_block(block)
  {
    m_socket = new ClientSocket(host, port);
  }

  // create a listener given a socket

  LuvListener::LuvListener(Socket* socket, const bool block)
    : ExecListener(),
      m_socket(socket),
      m_block(block)
  {
  }

  LuvListener::~LuvListener()
  {
    delete m_socket;
  }

  // handle node state transition event

  void LuvListener::sendPlanInfo() const
  {
    TiXmlElement planInfo(PLAN_INFO_STR);
    TiXmlElement block(VIEWER_BLOCKS_STR);
    block.InsertEndChild(TiXmlText(m_block ? TRUE_STR : FALSE_STR));
    planInfo.InsertEndChild(block);
    sendMessage(planInfo);
  }

  // handle node state transition event

  void LuvListener::notifyOfTransition(const LabelStr& /* prevState */, 
                                       const NodeId& node) const
  {
    // create update 

    TiXmlElement nodeStateUpdate(NODE_STATE_UPDATE_STR);

    // add state

    TiXmlElement state(NODE_STATE_STR);
    state.InsertEndChild(TiXmlText(node->getState().c_str()));
    nodeStateUpdate.InsertEndChild(state);

    // add outcome

    TiXmlElement outcome(NODE_OUTCOME_STR);
    outcome.InsertEndChild(TiXmlText(node->getOutcome().c_str()));
    nodeStateUpdate.InsertEndChild(outcome);

    // add failure type

    TiXmlElement failureType(NODE_FAILURE_TYPE_STR);
    failureType.InsertEndChild(TiXmlText(node->getFailureType().c_str()));
    nodeStateUpdate.InsertEndChild(failureType);
      
    // add the condition states
      
    TiXmlElement conditions(CONDITIONS_STR);
    nodeStateUpdate.InsertEndChild(constructConditions(conditions, node));


    // add the path
      
    TiXmlElement path(NODE_PATH_STR);
    constructNodePath(path, node);
    nodeStateUpdate.InsertEndChild(path);

    // send it off
      
    std::ostringstream buffer;
    buffer << nodeStateUpdate;
    sendMessage(nodeStateUpdate);
  }
   
  // handle add plan event

  void LuvListener::notifyOfAddPlan(const PlexilNodeId& plan,
                                    const LabelStr& /* parent */) const
  {
    // send an empty plan info

    sendPlanInfo();

    // create a plexil wrapper plan and stick the plan in it

    TiXmlElement planXml(PLEXIL_PLAN_STR);
    planXml.LinkEndChild(PlexilXmlParser::toXml(plan));

    // send plan to viewer
      
    sendMessage(planXml);
  }

  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A string naming the destination.
   * @param value The value (in internal Exec representation) being assigned.
   */
  void
  LuvListener::implementNotifyAssignment(const ExpressionId & dest,
					    const std::string& destName,
					    const double& value) const
  {
    if (m_socket != NULL) {
      TiXmlElement assignXml(ASSIGNMENT_TAG());
      TiXmlElement varXml(VARIABLE_TAG());
      const NodeId node = dest->getNode();
      if (node.isId())
	{
	  // get path to node
	  varXml.LinkEndChild(constructNodePath(NULL, node));
	}
      // get variable name
      TiXmlElement varName(VARIABLE_NAME_TAG());
      varName.InsertEndChild(*(new TiXmlText(destName)));

      varXml.InsertEndChild(varName);
      assignXml.InsertEndChild(varXml);

      // format variable value
      TiXmlElement * valXml = new TiXmlElement(VARIABLE_VALUE_TAG());
      valXml->InsertEndChild(*(new TiXmlText(valueToString(value))));
      assignXml.LinkEndChild(valXml);

      // send it to viewer
      sendMessage(assignXml);
    }
  }
   
  // handle add library event

  void LuvListener::notifyOfAddLibrary(const PlexilNodeId& plan) const
  {
    // send an empty plan info

    sendPlanInfo();

    // create a library wrapper and stick the plan in it

    TiXmlElement planXml(PLEXIL_LIBRARY_STR);
    planXml.LinkEndChild(PlexilXmlParser::toXml(plan));

    // send plan to viewer
      
    sendMessage(planXml);
  }


  //
  // Static member functions
  //
   
  // given a node id establish the path from the root to that node
   
  TiXmlNode& LuvListener::constructNodePath(TiXmlNode& path, 
                                            const NodeId& node)
  {
    if (node->getParent().isId())
      constructNodePath(path, node->getParent());
      
    TiXmlElement nodeId(NODE_ID_STR);
    nodeId.InsertEndChild(TiXmlText(node->getNodeId().toString()));
    path.InsertEndChild(nodeId);

    return path;
  }
   
  // given a node id establish the state of the conditions for this node
   
  TiXmlNode& LuvListener::constructConditions(TiXmlNode& conditions,
                                              const NodeId& node)
  {
    const std::set<double>& allConditions = node->ALL_CONDITIONS();
    for (std::set<double>::const_iterator 
           conditionName = allConditions.begin();
         conditionName != allConditions.end(); ++conditionName)
      {
        LabelStr name(*conditionName);
        TiXmlElement condition(name.toString());
        condition.InsertEndChild(TiXmlText(node->getCondition(name)->valueString()));
        conditions.InsertEndChild(condition);
      }

    return conditions;
  }


  //
  // Internal helper methods
  //

  // send a message to luv

  void LuvListener::sendMessage(const TiXmlNode& xml) const
  {
    std::ostringstream buffer;
    buffer << xml;
    debugMsg("LuvListener:sendMessage", " sending:\n" << buffer.str());
    sendMessage(buffer.str());
  }

  // send a string message to luv

  void LuvListener::sendMessage(const std::string& message) const
  {
    *m_socket << message << LUV_END_OF_MESSAGE;
    waitForAcknowledge();
  }

  // send a stream with a tag wrapped around it
   
  void LuvListener::sendTaggedStream(std::istream& stream, 
                                     const std::string& tag) const
  {
    char buffer[256];

    *m_socket << "<" << tag << ">";
    do
      {
        stream.read(buffer, sizeof(buffer));
        m_socket->send(buffer, stream.gcount());
      }
    while (!stream.eof());
    *m_socket << "</" << tag << ">";

    *m_socket << LUV_END_OF_MESSAGE;
    waitForAcknowledge();
  }

  // send the contents of an istream to luv

  void LuvListener::sendStream(std::istream& stream) const
  {
    char buffer[256];

    do
      {
        stream.read(buffer, sizeof(buffer));
        m_socket->send(buffer, stream.gcount());
      }
    while (!stream.eof());
      
    *m_socket << LUV_END_OF_MESSAGE;
    waitForAcknowledge();
  }

  // wait for luv to send back an acknowledgement

  void LuvListener::waitForAcknowledge() const
  {
    if (m_block)
      {
        std::string buffer;
         
        do
          {
            *m_socket >> buffer;
          }
        while (buffer[0] != LUV_END_OF_MESSAGE);
      }
  }

  //
    // Static member functions
    //

    // given a node id establish the path from the root to that node

    TiXmlNode* LuvListener::constructNodePath(TiXmlNode* path,
                                                 const NodeId& node)
    {
      // Construct path if not provided
      if (path == NULL)
        path = new TiXmlElement(NODE_PATH_TAG());

      if (node->getParent().isId())
        constructNodePath(path, node->getParent()); // for effect

      TiXmlElement* nodeId = new TiXmlElement(NODE_ID_TAG());
      nodeId->LinkEndChild(new TiXmlText(node->getNodeId().toString()));
      path->LinkEndChild(nodeId);

      return path;
    }

    // given a node id establish the state of the conditions for this node

    TiXmlNode* LuvListener::constructConditions(TiXmlNode* conditions,
                                                   const NodeId& node)
    {
      // Construct conditions element if not provided
      if (conditions == NULL)
        conditions = new TiXmlElement(CONDITIONS_TAG());

      const std::set<double>& allConditions = node->ALL_CONDITIONS();
      for (std::set<double>::const_iterator
             conditionName = allConditions.begin();
           conditionName != allConditions.end(); ++conditionName)
        {
          LabelStr name(*conditionName);
          TiXmlElement* condition = new TiXmlElement(name.toString());
          condition->LinkEndChild(new TiXmlText(node->getCondition(name)->valueString()));
          conditions->LinkEndChild(condition);
        }

      return conditions;
    }

    std::string valueToString(double val)
    {
		  if (LabelStr::isString(val))
			{
			  return std::string(LabelStr(val).toString());
			}
		  else if (StoredArray::isKey(val))
			{
			  return (StoredArray(val).toString());
			}
		  else
			{
				// assume it's a number instead
				// (do something special with ints?)
				std::ostringstream tmp;
				tmp << std::setprecision(15) << val;
				return tmp.str();
			}
    }
}
