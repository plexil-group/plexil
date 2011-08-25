/* -*- Mode: c++ -*-
 * File: plexil/src/interfaces/UdpAdapter/UdpAdapter.hh
 */

#include "InterfaceAdapter.hh"
#include "LabelStr.hh"
#include "MessageQueueMap.hh"
#include "udp-utils.hh"
#include "ThreadSpawn.hh"

class TiXmlElement;             // Forward references (w/o namespace)

namespace PLEXIL
{

  class Parameter
  {
  public:
    std::string desc;           // optional parameter description
    std::string type;           // int|float|bool|string
    int len;                    // 0 == variable length (i.e., char[])
  };

  class UdpMessage
  {
  public:
    std::string name;                // the Plexil Command name
    int len;                         // the length of the message in bytes
    std::list<Parameter> parameters; // message value parameters
    int local_port;                  // local port on which to receive
    std::string peer;                // peer to which to send
    int peer_port;                   // port to which to send
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
    //DECLARE_STATIC_CLASS_CONST(std::string, MESSAGE_PREFIX, "__MESSAGE__")
    //DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_PREFIX, "__LOOKUP__")
    //DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_ON_CHANGE_PREFIX, "__LOOKUP_ON_CHANGE__")
    DECLARE_STATIC_CLASS_CONST(std::string, PARAM_PREFIX, "__PARAMETER__")
    //DECLARE_STATIC_CLASS_CONST(std::string, SERIAL_UID_SEPARATOR, ":")

    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_MESSAGE_COMMAND, "SendMessage")
    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_UDP_MESSAGE_COMMAND, "SendUdpMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_UDP_MESSAGE_COMMAND, "ReceiveUdpMessage")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_MESSAGE_COMMAND, "ReceiveMessage")
    DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_COMMAND_COMMAND, "ReceiveCommand")
    DECLARE_STATIC_CLASS_CONST(LabelStr, GET_PARAMETER_COMMAND, "GetParameter")
    DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_RETURN_VALUE_COMMAND, "SendReturnValue")
    //DECLARE_STATIC_CLASS_CONST(LabelStr, UPDATE_LOOKUP_COMMAND, "UpdateLookup")

    // Constructor
    //UdpAdapter(AdapterExecInterface& execInterface);

    // Constructor/Destructor
    UdpAdapter(AdapterExecInterface& execInterface);
    UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml);
    virtual ~UdpAdapter();

    // InterfaceAdapter API
    bool initialize();
    bool start();
    bool stop();
    bool reset();
    bool shutdown();
    void registerChangeLookup(const LookupKey& uniqueId, const StateKey& stateKey, const std::vector<double>& tolerances);
    void unregisterChangeLookup(const LookupKey& uniqueId);
    void lookupNow(const StateKey& stateKey, std::vector<double>& dest);
    void sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack);
    // Executes a command with the given arguments
    void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    // Abort the given command with the given arguments.  Store the abort-complete into ack
    void invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    ThreadMutex m_cmdMutex;
    bool m_debug; // Show debugging output

    // Somewhere to hang the messages and default ports
    int m_default_local_port;
    int m_default_peer_port;
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
    void parseXmlMessageDefinitions(const TiXmlElement* xml);
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
