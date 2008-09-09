#ifndef RESPONSE_FACTORY_HH
#define RESPONSE_FACTORY_HH

#include <string>
#include <sstream>
#include <iostream>

template< class Type > bool parseType(std::istringstream& stringStream, Type& type)
{
  stringStream >> type;
  
  if( stringStream.fail() )
    {
      std::cerr << "Error: can not parse '" << stringStream << "'" << std::endl;
      
      return false;
    }
  
  return true;
}

class ResponseBase;

class ResponseFactory
{
public:
  ResponseFactory();
  virtual ~ResponseFactory();
  virtual ResponseBase* parse(const std::string& cmdName, timeval tDelay, 
                              std::istringstream& inStr);
private:
};

#endif // RESPONSE_FACTORY_HH
