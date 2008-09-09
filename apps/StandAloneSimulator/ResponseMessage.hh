#ifndef RESPONSE_MESSAGE_HH
#define RESPONSE_MESSAGE_HH

class ResponseMessage
{
public:
  ResponseMessage(int _id=-1, const std::string& _contents="")
    : id(_id), contents(_contents) {}
  virtual ~ResponseMessage(){}

  int id;
  std::string contents;
};

#endif // RESPONSE_MESSAGE_HH
