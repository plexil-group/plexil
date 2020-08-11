/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef SIMULATOR_HH
#define SIMULATOR_HH

#include "simdefs.hh" // ResponseManagerMap

class Agenda;
class CommandResponseManager;
class CommRelayBase;
struct ResponseMessage;

class Simulator
{
public:
  virtual ~Simulator()
  {
  }

  /**
   * @brief Starts a new background thread with the simulator top level loop and returns immediately.
   * @note Call only after reading all scripts.
   */
  virtual void start() = 0;

  /**
   * @brief Stops the simulator's top level thread.  Returns when the thread has rejoined.
   */
  virtual void stop() = 0;

  /**
   * @brief The simulator top level loop.  
   * @note Call only after reading all scripts.
   */
  virtual void simulatorTopLevel() = 0;

  //
  // API to comm relay
  //
  // These member functions are called in IPC thread context.
  //

  /**
   * @brief Schedules a response to the named command.
   * @param command The command name to which we are responding.
   * @param uniqueId Caller-specified identifier, passed through the simulator to the comm relay.
   */
  virtual void scheduleResponseForCommand(const std::string& command, 
                                          void* uniqueId = NULL) = 0;

  /**
   * @brief Get the current value of the named state.
   * @param stateName The state name to which we are responding.
   * @return Pointer to a ResponseMessage object, or NULL.
   */
  virtual ResponseMessage*
  getLookupNowResponse(const std::string& stateName, void* uniqueId) const = 0;

protected:

  Simulator()
  {
  }

};

// Simulator instance is responsible for deleting map and agenda
Simulator *makeSimulator(CommRelayBase *commRelay,
                         ResponseManagerMap *map,
                         Agenda *agenda);

#endif // SIMULATOR_HH
