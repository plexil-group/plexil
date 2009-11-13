/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "RoboSimResponseFactory.hh"
#include "RoboSimResponse.hh"
#include "GenericResponse.hh"

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
  else if (cmdName == "RobotState")
    {
      const int NUMBER_OF_STATE_READINGS=3;
      std::vector<double> state(NUMBER_OF_STATE_READINGS, 0.0);
      int i;
      for (i = 0; i < NUMBER_OF_STATE_READINGS; ++i)
        {
          double eLevel;
          if (parseType<double>(inStr, eLevel))
            state[i] = eLevel;
          else
            break;
        }
      if (i == NUMBER_OF_STATE_READINGS)
        return new RobotStateResponse(tDelay, state);
    }
  // fall-thru return
  return new GenericResponse(tDelay, std::vector<double>(1, 0.0));
}
