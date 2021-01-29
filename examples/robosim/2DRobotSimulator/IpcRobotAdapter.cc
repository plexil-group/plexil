// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "IpcRobotAdapter.hh"
#include "RobotBase.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ThreadSpawn.hh"

#include <sys/time.h>

//
// Local data types
//

//* @brief Map robot names to instances
using NameToRobotMap = std::map<const std::string, RobotBase *>;

//* @brief Unique identifier of a message sequence
using IpcMessageId = std::pair<std::string, uint32_t>;

//* brief Cache of not-yet-complete incoming message sequences
using IncompleteMessageMap = std::map<IpcMessageId, std::vector<PlexilMsgBase*> >;

//* @brief State name to unique ID map
using NameUniqueIDMap = std::map<std::string, IpcMessageId>;

class IpcRobotAdapterImpl final : public IpcRobotAdapter
{
private:

  // Helper class
  class MessageListener final : public PLEXIL::IpcMessageListener
  {
  public:
    MessageListener(IpcRobotAdapterImpl& adapter)
      : m_adapter(adapter)
    {
    }

    ~MessageListener() = default;

    void ReceiveMessage(const std::vector<PlexilMsgBase *>& msgs)
    {
      const PlexilMsgBase* leader = msgs[0];
      switch (leader->msgType) {
      case PlexilMsgType_Command:
        m_adapter.processCommand(msgs);
        break;

      case PlexilMsgType_LookupNow:
        m_adapter.processLookupNow(msgs);
        break;

      default:
        assertTrueMsg(ALWAYS_FAIL,
                      "ReceiveMessage: unimplemented leader message type "
                      << leader->msgType);
      }
    }
  private:
    IpcRobotAdapterImpl &m_adapter;
  };
  
  //
  // Member variables
  //

  //* @brief Map recording robot names and instances.
  NameToRobotMap m_robots;
  
  //* @brief Map from state name to unique ID of LookupOnChange request
  NameUniqueIDMap m_stateUIDMap;

  //* @brief Handler for the IPC connection
  PLEXIL::IpcFacade m_ipcFacade;

  //* @brief Message handler for IPC
  MessageListener m_listener;

public:

  /**
   * @brief Constructor. Opens the connection and spawns a listener thread.
   */
  IpcRobotAdapterImpl()
    : m_robots(),
      m_stateUIDMap(),
      m_ipcFacade(),
      m_listener(*this)
  {
  }

  /**
   * @brief Destructor.
   */
  virtual ~IpcRobotAdapterImpl() = default;

  virtual void initialize(const std::string &centralhost)
  {
    assertTrueMsg(IPC_OK == m_ipcFacade.initialize(m_ipcFacade.getUID().c_str(),
                                                   centralhost.c_str()),
                  "IpcRobotAdapter: Unable to initialize ipc to central server at "
                  << centralhost);

    // Spawn listener thread
    assertTrueMsg(m_ipcFacade.start() == IPC_OK,
                  "IpcRobotAdapter constructor: Unable to start IPC dispatch thread");

    // Subscribe only to messages we care about
    m_ipcFacade.subscribe(&m_listener, PlexilMsgType_Command);
    m_ipcFacade.subscribe(&m_listener, PlexilMsgType_LookupNow);
    debugMsg("IpcRobotAdapter:initialize", " succeeded");
  }

  virtual void stop()
  {
    m_ipcFacade.stop();
    debugMsg("IpcRobotAdapter:stop", " complete");
  }

  /**
   * @brief Make the adapter aware of the robot.
   */
  void registerRobot(const std::string& name, 
                                      RobotBase* robot)
  {
    NameToRobotMap::const_iterator it = m_robots.find(name);
    assertTrueMsg(it == m_robots.end(),
                  "Robot name conflict for \"" << name << "\"");
    m_robots[name] = robot;
  }
  /**
   * @brief Send a command to RoboSim
   */

  // N.B. RoboSim commands take one argument, the robot name.
  // Additional arguments are ignored.
  void processCommand(const std::vector<PlexilMsgBase *>& msgs)
  {
    PlexilStringValueMsg *cmdMsg = (PlexilStringValueMsg *) msgs[0];
    const std::string cmdName(cmdMsg->stringValue);
    assertTrueMsg(msgs[0]->count >= 1,
                  "processCommand: robot name argument missing for command \"" << cmdName << "\"");
    assertTrueMsg(msgs.size() >= 2,
                  "processCommand: internal error: not enough arguments to \""
                  << cmdName << "\" command");
    assertTrueMsg(msgs[1]->msgType == PlexilMsgType_StringValue,
                  "processCommand: robot name argument for command \"" << cmdName << "\" is not a string");
    PlexilStringValueMsg *nameMsg = (PlexilStringValueMsg *) msgs[1];
    const std::string robotName(nameMsg->stringValue);
    NameToRobotMap::const_iterator it = m_robots.find(robotName);
    assertTrueMsg(it != m_robots.end(),
                  "processCommand: no robot named \"" << robotName << "\"");
    RobotBase* robot = it->second;
    assertTrueMsg(robot != NULL,
                  "processCommand: robot named \"" << robotName << "\" is null!");
    IpcMessageId transId = IpcMessageId(msgs[0]->senderUID, msgs[0]->serial);
    double parameter = 0.0;
    // Check for missing parameter
    if (cmdName == "Move") {
      assertTrueMsg(msgs[0]->count >= 2,
                    "processCommand: missing required direction argument to \""
                    << cmdName << "\" command");
      checkError(msgs.size() >= 3,
                 "processCommand: internal error: not enough arguments to \""
                 << cmdName << "\" command");
      assertTrueMsg(msgs[2]->msgType == PlexilMsgType_IntegerValue,
                    "processCommand: direction argument for command \"" << cmdName << "\" is not an integer");
      parameter = ((const PlexilIntegerValueMsg*)msgs[2])->intValue;
      condDebugMsg(msgs[0]->count > 2,
                   "IpcRobotAdapter:processCommand",
                   "Ignoring " << msgs[0]->count - 2 << " argument(s)");
    }
    else {
      condDebugMsg(msgs[0]->count > 1,
                   "IpcRobotAdapter:processCommand",
                   "Ignoring " << msgs[0]->count - 1 << " argument(s)");
    }
    if (msgs[0]->count > 1) {
      debugMsg("IpcRobotAdapter:processCommand",
               " processing " << cmdName << '(' << robotName << ", " << parameter << ')');
    }
    else {
      debugMsg("IpcRobotAdapter:processCommand",
               " processing " << cmdName << '(' << robotName << ')');
    }
    m_ipcFacade.publishReturnValues(transId.second, transId.first,
                                    robot->processCommand(cmdName, parameter));
    debugMsg("IpcRobotAdapter:processCommand", " result published");
  }

  /**
   * @brief Deal with a LookupNow request
   */

  // N.B. RoboSim does not implement LookupNow
  void processLookupNow(const std::vector<PlexilMsgBase *>& msgs)
  {
    std::string stateName(((PlexilStringValueMsg *)msgs[0])->stringValue);
    debugMsg("IpcRobotAdapter:lookupNow", " ignoring lookup request for " << stateName);
  }

};

IpcRobotAdapter *makeIpcRobotAdapter()
{
  return new IpcRobotAdapterImpl();
}
