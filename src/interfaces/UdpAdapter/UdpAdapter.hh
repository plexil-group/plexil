/* -*- Mode: c++ -*-
 * File: plexil/src/interfaces/UdpAdapter/UdpAdapter.hh
 */

#include "InterfaceAdapter.hh"
#include "LabelStr.hh"

// Forward references w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  class UdpMessage
  {
  public:
    std::string name;
    std::string type;
    std::list<std::string> parameters;
    std::string host;
    int port;
    UdpMessage() : name(""), type(""), parameters(), host(""), port(0) {}
  };

  typedef std::map<std::string,UdpMessage> MessageMap;

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

    // Constructor
    //UdpAdapter(AdapterExecInterface& execInterface);

    // Constructor/Destructor
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
    // executes a command with the given arguments
    void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);
    // abort the given command with the given arguments.  store the abort-complete into ack
    void invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    // somewhere to hang the messages and default ports
    int m_default_outgoing_port;
    int m_default_incoming_port;
    MessageMap m_messages;

  private:
    // deliberately unimplemented
    UdpAdapter();
    UdpAdapter(const UdpAdapter &);
    UdpAdapter& operator=(const UdpAdapter &);

    //
    // Implementation methods
    //
    void executeSendMessageCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack);
   
    //
    // XML Support
    //
    void parseMessageDefinitions(const TiXmlElement* xml);
    void printMessageDefinitions();
  };
}

extern "C"
{
  void initUdpAdapter();
}
