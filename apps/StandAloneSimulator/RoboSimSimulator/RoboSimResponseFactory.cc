#include "RoboSimResponseFactory.hh"
#include "RoboSimResponse.hh"

RoboSimResponseFactory::RoboSimResponseFactory()
{
}

RoboSimResponseFactory::~RoboSimResponseFactory()
{
}

ResponseBase* RoboSimResponseFactory::parse(const std::string& cmdName, timeval tDelay,
                                            std::istringstream& inStr)
{
  if ((cmdName == "MoveUp") || (cmdName == "MoveRight") || (cmdName == "MoveDown") || 
      (cmdName == "MoveLeft"))
    {
      int returnValue;
      if (parseType<int>(inStr, returnValue))
        return new MoveResponse(tDelay, returnValue);
    }
  else if (cmdName == "QueryEnergySensor")
    {
      const int NUMBER_OF_ENERGY_LEVEL_READINGS=5;
      std::vector<double> energyLevel(NUMBER_OF_ENERGY_LEVEL_READINGS, 0.0);
      int i;
      for (i = 0; i < NUMBER_OF_ENERGY_LEVEL_READINGS; ++i)
        {
          double eLevel;
          if (parseType<double>(inStr, eLevel))
            energyLevel[i] = eLevel;
          else
            break;
        }
      if (i == NUMBER_OF_ENERGY_LEVEL_READINGS)
        return new QueryEnergyLevelResponse(tDelay, energyLevel);
    }
  
  // Should not reach the end unless to parse the input string
  return NULL;
}
