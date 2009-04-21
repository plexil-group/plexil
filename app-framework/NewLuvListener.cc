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

#include "NewLuvListener.hh"
#include "ClientSocket.h"
#include "tinyxml.h"

#include "Expression.hh"
#include "CoreExpressions.hh"
#include "Expressions.hh"

#include "Node.hh"

#include "CommonDefs.hh"
#include "PlexilExec.hh"
#include "PlexilPlan.hh"
#include "PlexilXmlParser.hh"
#include "LabelStr.hh"

#include <sstream>

// not currently used
//#define VULTURE_DELAY 10000

namespace PLEXIL 
{
  // create a listener given configuration XML

  NewLuvListener::NewLuvListener(const TiXmlElement* xml)
    : ExecListener(xml),
      m_socket(NULL),
      m_hostname(NULL),
      m_port(0),
      m_block(false)
  {
  }

  NewLuvListener::~NewLuvListener()
  {
    delete m_socket;
  }

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool NewLuvListener::initialize()
  {
    // parse XML to find host, port, blocking flag
    const TiXmlElement* xml = this->getXml();
    m_hostname = xml->Attribute(LUV_HOSTNAME_ATTR());
    if (m_hostname == NULL)
      {
        debugMsg("NewLuvListener:initialize",
                 " no " << LUV_HOSTNAME_ATTR()
                 << " attribute found, using default host " << LUV_DEFAULT_HOSTNAME());
        m_hostname = LUV_DEFAULT_HOSTNAME();
      }
    int rawPort = 0;
    const char* dummy = xml->Attribute(LUV_PORT_ATTR(), &rawPort);
    if (dummy == NULL)
      {
        debugMsg("NewLuvListener:initialize",
                 " no " << LUV_PORT_ATTR()
                 << " attribute found, using default port " << LUV_DEFAULT_PORT());
        m_port = LUV_DEFAULT_PORT();
      }
    else
      {
        // Should range check here
        // *** NYI ***
        m_port = (uint16_t) rawPort;
      }

    dummy = xml->Attribute(LUV_BLOCKING_ATTR());
    if (dummy == NULL)
      {
        debugMsg("NewLuvListener:initialize",
                 " no " << LUV_BLOCKING_ATTR()
                 << " attribute found, using default \"false\"");
        m_block = false;
      }
    else if (strcmp(dummy, TRUE_STR()) == 0)
      {
        m_block = true;
      }
    else
      {
        m_block = false;
      }
    return true; 
  }

  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool NewLuvListener::start() 
  { 
    m_socket = new ClientSocket(std::string(m_hostname), m_port);
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to stop.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool NewLuvListener::stop() 
  {
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to reset to initialized state.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool NewLuvListener::reset() 
  {
    delete m_socket;
    m_socket = NULL;
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to shut down.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool NewLuvListener::shutdown() 
  { 
    delete m_socket;
    m_socket = NULL;
    return true; 
  }

  // handle node state transition event

  void NewLuvListener::sendPlanInfo() const
  {
    TiXmlElement planInfo(PLAN_INFO_TAG());
    TiXmlElement block(VIEWER_BLOCKS_TAG());
    block.InsertEndChild(TiXmlText(m_block ? TRUE_STR() : FALSE_STR()));
    planInfo.InsertEndChild(block);
    sendMessage(planInfo);
  }

  // handle node state transition event

  void NewLuvListener::notifyOfTransition(const LabelStr& prevState, 
                                          const NodeId& node) const
  {
    // create update 

    TiXmlElement nodeStateUpdate(NODE_STATE_UPDATE_TAG());

    // add state

    TiXmlElement state(NODE_STATE_TAG());
    state.InsertEndChild(TiXmlText(node->getState().c_str()));
    nodeStateUpdate.InsertEndChild(state);

    // add outcome

    TiXmlElement outcome(NODE_OUTCOME_TAG());
    outcome.InsertEndChild(TiXmlText(node->getOutcome().c_str()));
    nodeStateUpdate.InsertEndChild(outcome);

    // add failure type

    TiXmlElement failureType(NODE_FAILURE_TYPE_TAG());
    failureType.InsertEndChild(TiXmlText(node->getFailureType().c_str()));
    nodeStateUpdate.InsertEndChild(failureType);
      
    // add the condition states
      
    TiXmlElement conditions(CONDITIONS_TAG());
    nodeStateUpdate.InsertEndChild(constructConditions(conditions, node));


    // add the path
      
    TiXmlElement path(NODE_PATH_TAG());
    constructNodePath(path, node);
    nodeStateUpdate.InsertEndChild(path);

    // send it off
      
    std::ostringstream buffer;
    buffer << nodeStateUpdate;
    sendMessage(nodeStateUpdate);
  }
   
  // handle add plan event

  void NewLuvListener::notifyOfAddPlan(const PlexilNodeId& plan,
                                       const LabelStr& parent) const
  {
    // send an empty plan info

    sendPlanInfo();

    // create a plexil wrapper plan and stick the plan in it

    TiXmlElement planXml(PLEXIL_PLAN_TAG());
    planXml.LinkEndChild(PlexilXmlParser::toXml(plan));

    // send plan to viewer
      
    sendMessage(planXml);
  }
   
  // handle add library event

  void NewLuvListener::notifyOfAddLibrary(const PlexilNodeId& plan) const
  {
    // send an empty plan info

    sendPlanInfo();

    // create a library wrapper and stick the plan in it

    TiXmlElement planXml(PLEXIL_LIBRARY_TAG());
    planXml.LinkEndChild(PlexilXmlParser::toXml(plan));

    // send plan to viewer
      
    sendMessage(planXml);
  }


  //
  // Static member functions
  //
   
  // given a node id establish the path from the root to that node
   
  TiXmlNode& NewLuvListener::constructNodePath(TiXmlNode& path, 
                                               const NodeId& node)
  {
    if (node->getParent().isId())
      constructNodePath(path, node->getParent());
      
    TiXmlElement nodeId(NODE_ID_TAG());
    nodeId.InsertEndChild(TiXmlText(node->getNodeId().toString()));
    path.InsertEndChild(nodeId);

    return path;
  }
   
  // given a node id establish the state of the conditions for this node
   
  TiXmlNode& NewLuvListener::constructConditions(TiXmlNode& conditions,
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

  void NewLuvListener::sendMessage(const TiXmlNode& xml) const
  {
    std::ostringstream buffer;
    buffer << xml;
    debugMsg("NewLuvListener:sendMessage", " sending:\n" << buffer.str());
    sendMessage(buffer.str());
  }

  // send a string message to luv

  void NewLuvListener::sendMessage(const std::string& message) const
  {
    *m_socket << message << LUV_END_OF_MESSAGE();
    waitForAcknowledge();
  }

  // send a stream with a tag wrapped around it
   
  void NewLuvListener::sendTaggedStream(std::istream& stream, 
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

    *m_socket << LUV_END_OF_MESSAGE();
    waitForAcknowledge();
  }

  // send the contents of an istream to luv

  void NewLuvListener::sendStream(std::istream& stream) const
  {
    char buffer[256];

    do
      {
        stream.read(buffer, sizeof(buffer));
        m_socket->send(buffer, stream.gcount());
      }
    while (!stream.eof());
      
    *m_socket << LUV_END_OF_MESSAGE();
    waitForAcknowledge();
  }

  // wait for luv to send back an acknowledgement

  void NewLuvListener::waitForAcknowledge() const
  {
    if (m_block)
      {
        std::string buffer;
         
        do
          {
            *m_socket >> buffer;
          }
        while (buffer[0] != LUV_END_OF_MESSAGE());
      }
  }
}
