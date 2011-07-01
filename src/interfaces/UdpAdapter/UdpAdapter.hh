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

#include "InterfaceAdapter.hh"
#include "LabelStr.hh"
//#include "ExecListener.hh"
//#include "AdapterExecInterface.hh"
//#include "ExecListener.hh"

//
// Forward references w/o namespace
//

class TiXmlElement;

namespace PLEXIL
{
  class UdpAdapter : public InterfaceAdapter
  {
  public:
    // Static Class Constants

    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_MESSAGE_COMMAND, "SendMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_MESSAGE_COMMAND, "ReceiveMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_COMMAND_COMMAND, "ReceiveCommand")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, GET_PARAMETER_COMMAND, "GetParameter")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_RETURN_VALUE_COMMAND, "SendReturnValue")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, UPDATE_LOOKUP_COMMAND, "UpdateLookup")

    /**
     * @brief Base constructor.
     */
    UdpAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor w/ configuration XML.
     */
    UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml);

    /**
     * @brief Destructor.
     */
    virtual ~UdpAdapter();

    /**
     * @brief Initialize and register the adapter, possibly using its configuration data.
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

    virtual void registerChangeLookup(const LookupKey& uniqueId, const StateKey& stateKey, const std::vector<double>& tolerances);

    virtual void unregisterChangeLookup(const LookupKey& uniqueId);

    virtual void lookupNow(const StateKey& stateKey, std::vector<double>& dest);

    virtual void sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack);

    // executes a command with the given arguments
    virtual void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    //abort the given command with the given arguments.  store the abort-complete into ack
    void invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

  private:
    // deliberately unimplemented
    UdpAdapter();
    UdpAdapter(const UdpAdapter &);
    UdpAdapter& operator=(const UdpAdapter &);

    //
    // Implementation methods
    //

    /**
     * @brief handles SEND_MESSAGE_COMMAND commands from the exec
     */
    void executeSendMessageCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);

  };
}
