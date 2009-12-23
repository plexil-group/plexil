/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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

#ifndef SAS_ADAPTER_HH
#define SAS_ADAPTER_HH

#include "ExecDefs.hh"
#include "InterfaceAdapter.hh"
#include <pthread.h>
#include <map>
#include <string.h>
#include "CommonDefs.hh"

extern "C" {
#include "ipc.h"
#include "ipcMsgFormats.h"
}

class IpcExecAdapter : public PLEXIL::InterfaceAdapter
{

public:
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  IpcExecAdapter(PLEXIL::AdapterExecInterface& execInterface);

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const pointer to the TiXmlElement describing this adapter
   * @note The instance maintains a shared pointer to the TiXmlElement.
   */
  IpcExecAdapter(PLEXIL::AdapterExecInterface& execInterface,
		   const TiXmlElement * xml);

  /**
   * @brief Destructor.
   */
  virtual ~IpcExecAdapter();

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  virtual bool initialize();

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  virtual bool start();

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  virtual bool stop();


  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   */
  virtual bool reset();

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  virtual bool shutdown();

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param stateKey The state key for this lookup.
   * @param dest A (reference to a) vector of doubles where the result is to be stored.
   */
  virtual void lookupNow(const PLEXIL::StateKey& stateKey,
                         std::vector<double>& dest);

  /**
   * @brief Register one LookupOnChange.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param tolerances A vector of tolerances for the LookupOnChange.
   */
  virtual void registerChangeLookup(const PLEXIL::LookupKey& uniqueId,
                                    const PLEXIL::StateKey& stateKey,
                                    const std::vector<double>& tolerances);

  /**
   * @brief Terminate one LookupOnChange.
   * @param uniqueId The unique ID of the lookup to be terminated.
   */
  virtual void unregisterChangeLookup(const PLEXIL::LookupKey& uniqueId);

  /**
   * @brief Execute a command with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the command.
   * @param ack The expression in which to store an acknowledgement of command transmission.
   */
  virtual void executeCommand(const PLEXIL::LabelStr& name, 
			      const std::list<double>& args,
                              PLEXIL::ExpressionId dest, 
			      PLEXIL::ExpressionId ack);


private:

  static std::string MessageIndicator, UniqueIDDivide;
  static int MessageIndicatorLength;
  pthread_t m_ThreadId;
  std::string myUniqueID;
  static std::vector<double> emptyArgListVector, trueVector;

  std::map<std::string, PLEXIL::ExpressionId> m_CommandToExpIdMap;
  
  //
  // Inner classes
  //

  std::string getMyIP();
  std::string getMyPID();

  static void strMsgHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			     void *clientData);
  
  void handleMsg(MSG_INSTANCE msgRef, BYTE_ARRAY callData);

  struct ChangeLookupStruct
  {
    ChangeLookupStruct(const PLEXIL::StateKey& _stateKey,
		       std::vector<double>& _prevValues,
		       const std::vector<double>& _tolerances)
      : stateKey(_stateKey), 
	prevValues(_prevValues),
	tolerances(_tolerances) 
    {
    }

    ~ChangeLookupStruct()
    {
    }

    inline const PLEXIL::StateKey& getStateKey() const 
    {
      return stateKey;
    }

    inline void setPreviousValues(std::vector<double>& _prevValues)
    { 
      prevValues = _prevValues;
    }

    inline const std::vector<double>& getPreviousValues() const 
    {
      return prevValues;
    }
    inline const std::vector<double>& getToleranceValues() const 
    {
      return tolerances;
    }

    const PLEXIL::StateKey stateKey;
    std::vector<double> prevValues;
    const std::vector<double> tolerances;
  };
};

#endif
