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
 *
 * IpcListener.cpp
 *
 *  Created on: Mar 1, 2010
 *      Author: jhogins
 */

#include "IpcListener.hh"
#include "AdapterExecInterface.hh"
#include "LabelStr.hh"
#include "tinyxml.h"
#include "Expression.hh"

namespace PLEXIL {

IpcListener::IpcListener(const TiXmlElement * xml) :
  ManagedExecListener(xml),
  m_assignedValues(new std::map<std::string, double>()),
  m_ipcFacade(new IpcFacade()),
  m_listener(*this){
}

IpcListener::~IpcListener() {
  delete m_assignedValues;
  delete m_ipcFacade;
}


bool IpcListener::initialize() {
  bool result = false;
  const char* taskName = NULL;
  const char* serverName = NULL;
  const TiXmlElement* xml = this->getXml();
  if (xml != NULL) {
    taskName = xml->Attribute("TaskName");
    serverName = xml->Attribute("Server");
  }
  if (taskName == NULL) {
    taskName = IpcFacade::getUID().c_str();
  }
  if (serverName == NULL) {
    serverName = "localhost";
  }
  result = m_ipcFacade->initilize(taskName, serverName);
  return result;
}

bool IpcListener::start() {
  bool res = true;
  res = m_ipcFacade->start();
  if (res == IPC_OK) {
    m_ipcFacade->subscribe(&m_listener, PlexilMsgType_LookupNow);
    debugMsg("IpcListener:start", "Succeeded in starting IPC");
    return true;
  } else {
    debugMsg("IpcListener:start", "Error in starting IPC");
    return false;
  }
}

bool IpcListener::stop() {
  m_ipcFacade->stop();
  return true;
}

bool IpcListener::reset() {
  return stop();
}

bool IpcListener::shutdown() {
  m_ipcFacade->shutdown();
  return true;
}
/**
 * @brief Notify that a variable assignment has been performed.
 * @param dest The Expression being assigned to.
 * @param destName A string naming the destination.
 * @param value The value (in internal Exec representation) being assigned.
 */
void IpcListener::implementNotifyAssignment(const ExpressionId & dest,
                                const std::string& destName,
                                const double& value) const {
  (*m_assignedValues)[destName] = value;
  assertTrueMsg(m_ipcFacade->publishTelemetry(destName, std::list<double>(1, value)) != IpcFacade::ERROR_SERIAL(),
      "IpcListener:implementNotifyAssignment: publishTelemetry returned status \""
      << m_ipcFacade->getError() << "\"");
}

/**
 * @brief Handler function as seen by IPC.
 */

// *** TODO: confirm correct message formats!! ***

IpcListener::IpcListenerMsgListener::IpcListenerMsgListener(const IpcListener& parent) :
    m_parent(parent) {}

void IpcListener::IpcListenerMsgListener::ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs) {
  if (strcmp(msgs.front()->senderUID, IpcFacade::getUID().c_str()) == 0) {
    debugMsg("IpcListener:receiveMessage", " ignoring my own outgoing message");
    return;
  }

  debugMsg("IpcListener:receiveMessage", " received message. processing as LookupNow");
  const PlexilStringValueMsg* msg = reinterpret_cast<const PlexilStringValueMsg*>(msgs.front());
  std::map<std::string, double>::iterator it = m_parent.m_assignedValues->find(msg->stringValue);
  if (it == m_parent.m_assignedValues->end()) {
    debugMsg("IpcListener:receiveMessage", " unassigned variable " << msg->stringValue
        << ", returning UNKNOWN");
    m_parent.m_ipcFacade->publishReturnValues(
        msg->header.serial, msg->header.senderUID,
        std::list<double>(1, Expression::UNKNOWN()));
  } else {
    debugMsg("IpcListener:receiveMessage", " assigned variable " << msg->stringValue
        << ", returning " << (*it).second);
    m_parent.m_ipcFacade->publishReturnValues(
        msg->header.serial, msg->header.senderUID,
        std::list<double>(1, (*it).second));
  }
}
}
