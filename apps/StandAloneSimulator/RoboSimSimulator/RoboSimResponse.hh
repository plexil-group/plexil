#ifndef ROBOSIM_RESPONSE_HH
#define ROBOSIM_RESPONSE_HH

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "ResponseBase.hh"
#include "ResponseMessage.hh"

class MoveResponse : public ResponseBase
{
public:
  MoveResponse(timeval delay, const int returnValue)
    : ResponseBase(delay), m_ReturnValue(returnValue) {}

  ~MoveResponse(){}

  virtual ResponseMessage* createResponseMessage()
  {
    std::cout << "Creating a Move response: " << m_ReturnValue << std::endl;
    std::ostringstream str;
    str << m_ReturnValue;
    return new ResponseMessage(-1, str.str());
  }

private:
  const int m_ReturnValue;
};


class QueryEnergyLevelResponse : public ResponseBase
{
public:
  QueryEnergyLevelResponse(timeval delay, const std::vector<double> energyLevel) 
    : ResponseBase(delay), m_EnergyLevel(energyLevel) {}

  ~QueryEnergyLevelResponse(){}

  virtual ResponseMessage* createResponseMessage()
  {
    std::ostringstream str;
    for(unsigned int i = 0; i < m_EnergyLevel.size(); ++i)
      {
        str << m_EnergyLevel[i];
        if (i < (m_EnergyLevel.size() - 1))
          str << ",";
      }
      
    return new ResponseMessage(-1, str.str());
  }

private:
  const std::vector<double> m_EnergyLevel;
};

#endif //ROBOSIM_RESPONSE_HH
