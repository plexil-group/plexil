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
 * IpcListener.h
 *
 *  Created on: Mar 1, 2010
 *      Author: jhogins
 */

#ifndef IPCLISTENER_H_
#define IPCLISTENER_H_

#include "ManagedExecListener.hh"
#include "IpcFacade.hh"

#include <map>
#include <string>

namespace PLEXIL
{
class IpcListener : public ManagedExecListener {
public:
  IpcListener(const TiXmlElement * xml);
  virtual ~IpcListener();

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   */
  bool initialize();

  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool start();

  /**
   * @brief Perform listener-specific actions to stop.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool stop();

  /**
   * @brief Perform listener-specific actions to reset to initialized state.
   * @return true if successful, false otherwise.
   */
  bool reset();

  /**
   * @brief Perform listener-specific actions to shut down.
   * @return true if successful, false otherwise.
   */
  bool shutdown();

protected:
  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A string naming the destination.
   * @param value The value (in internal Exec representation) being assigned.
   */
  virtual void implementNotifyAssignment(const ExpressionId & dest,
                                         const std::string& destName,
                                         const double& value) const;
private:

  /**
   * Message Listener to register with IpcFacade
   */
  class IpcListenerMsgListener : public IpcMessageListener {
  public:
    IpcListenerMsgListener(const IpcListener& parent);
    void ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs);
  private:
    const IpcListener& m_parent;
  };
  //* @brief Listener instance to receive messages.
  IpcListenerMsgListener m_listener;
  //* @brief Interface for sending/receiving messages through IPC.
  IpcFacade* const m_ipcFacade;
  //* @brief Map of latest assigned values of internal variables for lookups.
  std::map<std::string, double>* const m_assignedValues;
};
}
#endif /* IPCLISTENER_H_ */
