/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#ifndef IPC_COMM_RELAY_HH
#define IPC_COMM_RELAY_HH

#include "CommRelayBase.hh"
#include "ipc-data-formats.h"
#include "IpcFacade.hh"

#include <ipc.h>

#include <map>
#include <string>
#include <vector>

#include <pthread.h>

// forward references
class ResponseMessage;

class IpcCommRelay : public CommRelayBase
{
public:

  /**
   * @brief Constructor. Opens the connection and spawns a listener thread.
   */
  IpcCommRelay(const std::string& id, const std::string& centralhost = "localhost:1381");

  /**
   * @brief Destructor. Shuts down the listener thread and closes the connection.
   */
  ~IpcCommRelay();

  /**
   * @brief Send a response from the sim back to the UE.
   */
  void sendResponse(const ResponseMessage* respMsg);

private:

  //
  // Deliberately unimplemented
  //
  IpcCommRelay();
  IpcCommRelay(const IpcCommRelay&);
  IpcCommRelay& operator=(const IpcCommRelay&);

  //
  // Implementation methods
  //

  /**
   * @brief Send a command to the simulator
   */
  void processCommand(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Deal with a LookupNow request
   */
  void processLookupNow(const std::vector<const PlexilMsgBase*>& msgs);

  //* brief Class to receive messages from Ipc
  class MessageListener : public PLEXIL::IpcMessageListener  {
  public:
    MessageListener(IpcCommRelay&);
    ~MessageListener();
    void ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs);
  private:
    IpcCommRelay& m_adapter;
  };

  //
  // Private data types
  //

  //* @brief Unique identifier of a message sequence
  typedef std::pair<std::string, uint32_t> IpcMessageId;

  //* brief Cache of not-yet-complete incoming message sequences
  typedef std::map<IpcMessageId, std::vector<const PlexilMsgBase*> > IncompleteMessageMap;

  //* @brief State name to unique ID map
  typedef std::map<std::string, IpcMessageId> NameUniqueIDMap;

  //
  // Member variables
  //

  //* @brief Handler for the IPC connection
  PLEXIL::IpcFacade m_ipcFacade;
  
  //* @brief Map from state name to unique ID of LookupOnChange request
  NameUniqueIDMap m_stateUIDMap;

  //* @brief Message handler for IPC
  MessageListener m_listener;

};

#endif // IPC_COMM_RELAY_HH
