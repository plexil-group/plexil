#ifndef GENERIC_RESPONSE_HH
#define GENERIC_RESPONSE_HH

#include <string>
#include <iostream>
#include "ResponseBase.hh"
#include "ResponseMessage.hh"

class GenericResponse : public ResponseBase
{
public:
  GenericResponse(timeval delay) 
    : ResponseBase(delay)
  {

  }

  ~GenericResponse(){}

  virtual ResponseMessage* createResponseMessage()
  {
    std::cout << "Creating a generic response." << std::endl;

    return new ResponseMessage(0, "0");
  }

private:
};

#endif //GENERIC_RESPONSE_HH
