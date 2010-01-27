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

#include <iostream>
#include <LabelStr.hh>
#include <StoredArray.hh>

#include "RoboSimInterfaceAdapter.hh"
#include "CoreExpressions.hh"
#include "InterfaceSchema.hh"
#include "tinyxml.h"
#include "AdapterExecInterface.hh"

#define EXPORT __attribute__((visibility("default")))

EXPORT
RoboSimInterfaceAdapter::RoboSimInterfaceAdapter(PLEXIL::AdapterExecInterface& execInterface, const TiXmlElement*& configXml) :
    InterfaceAdapter(execInterface, configXml),
    m_Name(configXml->Attribute(PLEXIL::InterfaceSchema::ADAPTER_TYPE_ATTR())),
    m_SSWGClient(),
    m_Connected(false) {
  const TiXmlNode* ipNode = configXml->FirstChild(PLEXIL::InterfaceSchema::IP_ADDRESS_TAG());
  if (ipNode) {
    const TiXmlElement *ipElem = ipNode->ToElement();
    if (ipElem) {
      m_IpAddress = ipElem->GetText();
    }
  }
  if (!m_IpAddress){
    m_IpAddress = DEFAULT_IP_ADDRESS();
  }
  const TiXmlNode* portNode = configXml->FirstChild(PLEXIL::InterfaceSchema::PORT_NUMBER_TAG());
  if (portNode) {
    const TiXmlElement *portElem = portNode->ToElement();
    if (portElem) {
      std::stringstream ss(portElem->GetText());
      int num;
      if((ss >> num).fail()) {
          m_PortNumber = DEFAULT_PORT();
      } else {
        m_PortNumber = num;
      }
    }
  } else {
    m_PortNumber = DEFAULT_PORT();
  }
  m_MoveDirectionVector.push_back("MoveUp");
  m_MoveDirectionVector.push_back("MoveRight");
  m_MoveDirectionVector.push_back("MoveDown");
  m_MoveDirectionVector.push_back("MoveLeft");
}

RoboSimInterfaceAdapter::~RoboSimInterfaceAdapter() {
}

/**
 * @brief Initializes the adapter, possibly using its configuration data.
 * @return true if successful, false otherwise.
 */
bool RoboSimInterfaceAdapter::initialize() {
  m_execInterface.defaultRegisterAdapter(getId());
  return true;
}

/**
 * @brief Starts the adapter, possibly using its configuration data.  
 * @return true if successful, false otherwise.
 */
bool RoboSimInterfaceAdapter::start() {
  m_Connected = m_SSWGClient.connectToServer(m_Name, m_IpAddress, m_PortNumber, this);
  if (!m_Connected) {
    std::cerr << "The executive is unable to connect to the " << "communication server" << std::endl;
    return false;
  }
  std::cout << "Successfully connected to server." << std::endl;
  return true;
}

/**
 * @brief Stops the adapter.  
 * @return true if successful, false otherwise.
 */
bool RoboSimInterfaceAdapter::stop() {
  return true;
}

/**
 * @brief Resets the adapter.  
 * @return true if successful, false otherwise.
 */
bool RoboSimInterfaceAdapter::reset() {
  return true;
}

/**
 * @brief Shuts down the adapter, releasing any of its resources.
 * @return true if successful, false otherwise.
 */
bool RoboSimInterfaceAdapter::shutdown() {
  return true;
}

void RoboSimInterfaceAdapter::lookupNow(const PLEXIL::StateKey& stateKey, std::vector<double>& dest) {
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdapter", "Received LookupNow for "
      << nStr);
  dest.resize(1);

  if ((m_LookupMapIter = m_LookupMap.find(nStr)) != m_LookupMap.end()) {
    // The state is available at this time
    std::vector<double> retValues = m_LookupMapIter->second;
    if (retValues.size() > 1) {
      // return as an array
      PLEXIL::StoredArray retArray(retValues.size(), retValues);
      dest[0] = retArray.getKey();
    } else {
      dest[0] = retValues[0];
    }
    m_LookupMap.erase(m_LookupMapIter);
  } else {
    // The state is not available at this time
    debugMsg("RoboSimInterfaceAdapter", "State not available for " << nStr);
    dest[0] = PLEXIL::Expression::UNKNOWN();
  }
}

void RoboSimInterfaceAdapter::executeCommand(const PLEXIL::LabelStr& name, const std::list<double>& args, PLEXIL::ExpressionId dest, PLEXIL::ExpressionId ack) {
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdapter", "Received executeCommand for "
      << nStr);
  bool failed = false;
  if (nStr == "Move") {
    int direction = static_cast<int> (*(args.begin()));
    if ((direction >= 0) && (direction < static_cast<int> (m_MoveDirectionVector.size())))
      nStr = m_MoveDirectionVector[direction];
    else {
      debugMsg("RoboSimInterfaceAdapter", "The specified move direction is not "
          << "valid. The command will fail");
      failed = true;
    }
  }

  if (m_Connected && !failed) {
    m_CommandMap[nStr] = dest;
    m_SSWGClient.sendMessage(nStr, "RobotYellow");
  }

  m_execInterface.handleValueChange(ack, (!m_Connected || failed) ? PLEXIL::CommandHandleVariable::COMMAND_FAILED()
      : PLEXIL::CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
  m_execInterface.notifyOfExternalEvent();
}

void RoboSimInterfaceAdapter::executeFunctionCall(const PLEXIL::LabelStr& name, const std::list<double>& args, PLEXIL::ExpressionId dest,
    PLEXIL::ExpressionId ack) {
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdapter", "Received functionCall for "
      << nStr);
  std::list<double>::const_iterator argIter = args.begin();

  double minVal = (*argIter);
  ++argIter;
  double maxVal = (*argIter);
  int res = static_cast<int> ((minVal + (maxVal - minVal) * (static_cast<double> (rand()) / static_cast<double> (RAND_MAX))) + 0.5);

  m_execInterface.handleValueChange(ack, PLEXIL::BooleanVariable::TRUE());
  m_execInterface.handleValueChange(dest, res);
  m_execInterface.notifyOfExternalEvent();
}

void RoboSimInterfaceAdapter::receivedMessage(const std::string& sender, const std::string& msg) {
  debugMsg("RoboSimInterfaceAdapter", "got something back from " << sender
      << " who says: " << msg);

  std::string cmdName;
  std::vector<double> retValues = extractMessageContents(cmdName, msg);

  if ((m_CommandMapIter = m_CommandMap.find(cmdName)) != m_CommandMap.end()) {
    if (retValues.size() > 1) {
      // return as an array
      PLEXIL::StoredArray retArray(retValues.size(), retValues);
      m_execInterface.handleValueChange(m_CommandMapIter->second, retArray.getKey());
    } else {
      m_execInterface.handleValueChange(m_CommandMapIter->second, retValues[0]);
    }
    m_CommandMap.erase(m_CommandMapIter);
  } else {
    debugMsg("RoboSimInterfaceAdapter" , "Received an unsolicited reply for "
        << cmdName << ". Ignoring it.");
    return;
  }

  m_execInterface.notifyOfExternalEvent();
}

std::vector<double> RoboSimInterfaceAdapter::extractMessageContents(std::string& cmdName, const std::string& msg) {
  std::vector<double> retValues;
  std::string delimiter(",");
  std::string::size_type lastPos = msg.find_first_not_of(delimiter, 0);
  std::string::size_type pos = msg.find_first_of(delimiter, lastPos);
  cmdName = msg.substr(lastPos, pos - lastPos);
  bool done = false;
  while (!done) {
    lastPos = msg.find_first_not_of(delimiter, pos);
    pos = msg.find_first_of(delimiter, lastPos);
    if ((std::string::npos == pos) && (std::string::npos == lastPos)) {
      done = true;
    } else {
      retValues.push_back(atof(msg.substr(lastPos, pos - lastPos).c_str()));
    }
  }
  debugMsg("RoboSimInterfaceAdapter", "Extracting the following values for "
      << cmdName << ", retValue size: " << retValues.size());
  for (unsigned int i = 0; i < retValues.size(); ++i)
    debugMsg("RoboSimInterfaceAdapter", retValues[i]);

  return retValues;
}
