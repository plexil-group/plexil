#ifndef ROBOSIM_RESPONSE_FACTORY
#define ROBOSIM_RESPONSE_FACTORY

#include "ResponseFactory.hh"

class RoboSimResponseFactory : public ResponseFactory
{
public:
  RoboSimResponseFactory();
  ~RoboSimResponseFactory();

  virtual ResponseBase* parse(const std::string& cmdName, timeval tDelay, 
                              std::istringstream& inStr);
};
#endif // ROBOSIM_RESPONSE_FACTORY
