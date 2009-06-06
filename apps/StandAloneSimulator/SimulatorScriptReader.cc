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
#include "SimulatorScriptReader.hh"
#include "Simulator.hh"
#include "ResponseMessageManager.hh"
#include "ResponseBase.hh"
#include "ResponseFactory.hh"

#include <fstream>
#include <iostream>
#include <sstream>

SimulatorScriptReader::SimulatorScriptReader(Simulator* simulator)
  : m_Simulator(simulator)
{
}

SimulatorScriptReader::~SimulatorScriptReader()
{
}

bool SimulatorScriptReader::readCommandScript(const std::string& fName)
{
  return readScript(fName);
}

bool SimulatorScriptReader::readTelemetryScript(const std::string& fName)
{
  return readScript(fName, true);
}

bool SimulatorScriptReader::readScript(const std::string& fName,
                                       bool telemetry)
{
  std::ifstream inputFile( fName.c_str());
  
  if ( !inputFile )
    {
      std::cerr << "Error: cannot open script file '" << fName <<"'" << std::endl;
      return false;
    }
  
  int lineCount = 0;

  const int MAX_INPUT_LINE_LENGTH = 1024;
  
  //1st line for a given message
  std::string commandName;
  long commandIndex=0;
  double delay;
  int numOfResponses;

  while ( !inputFile.eof() ) 
    {
      char inLine[MAX_INPUT_LINE_LENGTH];
      
      inputFile.getline( inLine, MAX_INPUT_LINE_LENGTH );
      
      lineCount++;
      
      while (!isalpha (inLine[0]) && !inputFile.eof() ) 
        {
          inputFile.getline (inLine, MAX_INPUT_LINE_LENGTH);
          
          lineCount++;
        }
      
      if( inputFile.eof() )
        {
          return true;
        } 
      
      std::istringstream inputStringStream( inLine );
      
      inputStringStream >> commandName;

      if (telemetry)
        {
          numOfResponses = 1;
          inputStringStream >> delay;
          //          ++commandIndex;
        }
      else
        {
          inputStringStream >> commandIndex;
          inputStringStream >> numOfResponses;
          inputStringStream >> delay;
        }

      std::cout << "\nRead a new line for \"" << commandName
                << "\", delay = " << delay << std::endl;
      ResponseMessageManager* responseMessageManager = 
        m_Simulator->getResponseMessageManager(commandName);
      
      if( 0 == responseMessageManager )
        {
          std::cout << "Creating a message manager for "
                    << (telemetry ? "telemetry" : "command")
                    << " \"" << commandName 
                    << "\"" << std::endl;
          responseMessageManager = new ResponseMessageManager(commandName);
	  
          m_Simulator->registerResponseMessageManager(responseMessageManager);

          if (telemetry) 
            commandIndex = 1;
        }
      else 
        if (telemetry) 
          ++commandIndex;


      inputFile.getline( inLine, MAX_INPUT_LINE_LENGTH );
      lineCount++;

      if( inputFile.eof())
        {
          std::cerr << "Error: response line missing in script-file " << fName 
                    << " at line " << lineCount << std::endl;
          
          return false;
        }
      
      std::istringstream responseStringStream( inLine );
      
      timeval timeDelay = m_Simulator->convertDoubleToTimeVal(delay);
      
      ResponseBase* response = 
        m_Simulator->getResponseFactory()->parse(commandName, timeDelay,
                                                 responseStringStream);
      
      std::cout << "Command Index: " << commandIndex << std::endl;
      if(response != 0)
        {
          response->setNumberOfResponses(numOfResponses);
          responseMessageManager->addResponse(commandIndex, response);
        }
      else
        {
          std::cout << "ERROR: Unable to parse response for \""
                    << commandName
                    << "\" at line "
                    << lineCount
                    << "\nResponse line was: "
                    << inLine
                    << std::endl;
          return false;
        }
      
      if (telemetry)
        {
          m_Simulator->scheduleResponseForTelemetry(commandName);
        }
    }
  
  inputFile.close();

  return true;
}
