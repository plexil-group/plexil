#include "ResponseFactory.hh"
#include "GenericResponse.hh"

ResponseFactory::ResponseFactory()
{
}

ResponseFactory::~ResponseFactory()
{
}

ResponseBase* ResponseFactory::parse(const std::string& cmdName, timeval tDelay,
                                     std::istringstream& inStr)
{
  return new GenericResponse(tDelay);
}
