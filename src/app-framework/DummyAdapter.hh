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

#ifndef DUMMY_ADAPTER_H
#define DUMMY_ADAPTER_H

#include "InterfaceAdapter.hh"

//
// Forward references w/o namespace
//

class TiXmlElement;

namespace PLEXIL
{
  /*!
    \brief A dummy InterfaceAdapter for testing purposes
  */
  class DummyAdapter : public InterfaceAdapter
  {
  public:

    /**
     * @brief Base constructor.
     */
    DummyAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor w/ configuration XML.
     */
    DummyAdapter(AdapterExecInterface& execInterface,
                 const TiXmlElement* xml);

    /**
     * @brief Destructor.
     */
    ~DummyAdapter();

    /**
     * @brief Initialize and register the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     */
    bool start();

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     */
    bool stop();

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     */
    bool reset();

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     */
    bool shutdown();

    virtual void registerChangeLookup(const LookupKey& uniqueId,
				      const StateKey& stateKey,
				      const std::vector<double>& tolerances);

    virtual void unregisterChangeLookup(const LookupKey& uniqueId);

    void lookupNow(const StateKey& stateKey,
		   std::vector<double>& dest);

    void sendPlannerUpdate(const NodeId& node,
			   const std::map<double, double>& valuePairs,
			   ExpressionId ack);

    // executes a command with the given arguments
    void executeCommand(const LabelStr& name,
			const std::list<double>& args,
			ExpressionId dest,
			ExpressionId ack);

    //abort the given command with the given arguments.  store the abort-complete into ack
    void invokeAbort(const LabelStr& name, 
		     const std::list<double>& args, 
		     ExpressionId dest,
		     ExpressionId ack);
    
  private:
    // deliberately unimplemented
    DummyAdapter();
    DummyAdapter(const DummyAdapter &);
    DummyAdapter& operator=(const DummyAdapter &);

  };

}

#endif // DUMMY_ADAPTER_H
