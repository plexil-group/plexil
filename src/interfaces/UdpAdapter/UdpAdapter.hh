/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
#include "MessageQueueMap.hh"
#include "udp-utils.hh"
#include "ThreadSpawn.hh"

namespace PLEXIL
{

  class Parameter
  {
  public:
    std::string desc;           // optional parameter description
    std::string type;           // int|float|bool|string|int-array|float-array|string-array|bool-array
    unsigned int len;           // number of bytes for type (or array element)
    unsigned int elements;      // number of elements in the array (non-array types are 0 or 1?)
  };

  class UdpMessage
  {
  public:
    std::string name;                // the Plexil Command name
    unsigned int len;                         // the length of the message in bytes
    std::list<Parameter> parameters; // message value parameters
    unsigned int local_port;                  // local port on which to receive
    std::string peer;                // peer to which to send
    unsigned int peer_port;                   // port to which to send
    int sock;                        // socket to use -- only meaningful in call to waitForUdpMessage
    void* self;                      // reference to the UdpAdapter for use in message decoding
    UdpMessage() : name(), len(0), parameters(), peer(""), local_port(0), peer_port(0), sock(0), self(NULL) {}
  };

  typedef std::map<std::string, UdpMessage> MessageMap;
  typedef std::map<std::string, pthread_t> ThreadMap;
  typedef std::map<std::string, int> SocketMap;

  class UdpAdapter : public InterfaceAdapter
  {
  public:
    // Static Class Constants
    DECLARE_STATIC_CLASS_CONST(std::string, COMMAND_PREFIX, "__COMMAND__")
    DECLARE_STATIC_CLASS_CONST(std::string, PARAM_PREFIX, "__PARAMETER__")
    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_MESSAGE_COMMAND, "SendMessage")
    DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_COMMAND_COMMAND, "ReceiveCommand")
    DECLARE_STATIC_CLASS_CONST(LabelStr, GET_PARAMETER_COMMAND, "GetParameter")
    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_RETURN_VALUE_COMMAND, "SendReturnValue")
    //DECLARE_STATIC_CLASS_CONST(std::string, MESSAGE_PREFIX, "__MESSAGE__")
    //DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_PREFIX, "__LOOKUP__")
    //DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_ON_CHANGE_PREFIX, "__LOOKUP_ON_CHANGE__")
    //DECLARE_STATIC_CLASS_CONST(std::string, SERIAL_UID_SEPARATOR, ":")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_UDP_MESSAGE_COMMAND, "SendUdpMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_UDP_MESSAGE_COMMAND, "ReceiveUdpMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_MESSAGE_COMMAND, "ReceiveMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, UPDATE_LOOKUP_COMMAND, "UpdateLookup")

    // Constructor/Destructor
    UdpAdapter(AdapterExecInterface& execInterface);
    UdpAdapter(AdapterExecInterface& execInterface, const pugi::xml_node& xml);
    virtual ~UdpAdapter();

    // InterfaceAdapter API
    bool initialize();
    bool start();
    bool stop();
    bool reset();
    bool shutdown();
    double lookupNow(const State& stateKey);
	void subscribe(const State& state);
	void unsubscribe(const State& state);
    void sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack);
    // Executes a command with the given arguments
    void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    // Abort the given command with the given arguments.  Store the abort-complete into ack
    void invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    ThreadMutex m_cmdMutex;
    bool m_debug; // Show debugging output

    // Somewhere to hang the messages, default ports and peers, threads and sockets
    unsigned int m_default_local_port;
    unsigned int m_default_peer_port;
    std::string m_default_peer;
    MessageMap m_messages;
    MessageQueueMap m_messageQueues;
    ThreadMap m_activeThreads;
    SocketMap m_activeSockets;

  private:
    // Deliberately unimplemented
    UdpAdapter();
    UdpAdapter(const UdpAdapter &);
    UdpAdapter& operator=(const UdpAdapter &);

    //
    // Implementation methods
    //
    void executeSendUdpMessageCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeReceiveUdpCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeSendMessageCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeReceiveCommandCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeGetParameterCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeSendReturnValueCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    void executeDefaultCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);
   
    //
    // XML Support
    //
    void parseXmlMessageDefinitions(const pugi::xml_node& xml);
    void printMessageDefinitions();
    int buildUdpBuffer(unsigned char* buffer,
                       const UdpMessage& msg,
                       const std::list<double>& args,
                       bool skip_arg=false,
                       bool debug=false);
    void printMessageContent(const LabelStr& name, const std::list<double>& args);
    int sendUdpMessage(const unsigned char* buffer, const UdpMessage& msg, bool debug=false);
    int startUdpMessageReceiver(const LabelStr& name, ExpressionId dest, ExpressionId ack);
    static void* waitForUdpMessage(UdpMessage* msg);
    int handleUdpMessage(const UdpMessage* msg, const unsigned char* buffer, bool debug=false);
    double formatMessageName(const LabelStr& name, const LabelStr& command, int id);
    double formatMessageName(const LabelStr& name, const LabelStr& command);
    double formatMessageName(const char* name, const LabelStr& command);
 };
}

extern "C"
{
  void initUdpAdapter();
}
