#ifndef RESPONSE_BASE_HH
#define RESPONSE_BASE_HH

class ResponseMessage;

class ResponseBase
{
public:
  ResponseBase(timeval delay) : m_Delay(delay) {}
  virtual ~ResponseBase(){}

  timeval getDelay() const {return m_Delay;}

  virtual ResponseMessage* createResponseMessage() = 0;

private:
  timeval m_Delay;
};

#endif // RESPONSE_BASE_HH
