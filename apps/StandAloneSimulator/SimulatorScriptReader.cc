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

bool SimulatorScriptReader::readScript(const std::string& fName)
{
  std::ifstream inputFile( fName.c_str());
  
  if ( !inputFile )
    {
      std::cerr << "Error: can not open script file '" << fName <<"'" << std::endl;
      return false;
    }
  
  int lineCount = 0;

  const int MAX_INPUT_LINE_LENGTH = 1024;
  
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
      
      //1st line for a given message
      std::string commandName;
      long commandIndex;
      double delay;
      int numOfResponses;
      
      std::istringstream inputStringStream( inLine );
      
      inputStringStream >> commandName;
      inputStringStream >> commandIndex;
      inputStringStream >> numOfResponses;
      inputStringStream >> delay;

      ResponseMessageManager* responseMessageManager = 
        m_Simulator->getResponseMessageManager(commandName);
      
      if( 0 == responseMessageManager )
        {
          responseMessageManager = new ResponseMessageManager(commandName);
	  
          m_Simulator->registerResponseMessageManager(responseMessageManager);
        }

      if( numOfResponses > 0 )
        {
          inputFile.getline( inLine, MAX_INPUT_LINE_LENGTH );
          lineCount++;
	  
          if( inputFile.eof())
            {
              std::cerr << "Error: response line missing in script-file " << fName 
                        << " expecting one at line " << lineCount << std::endl;
              
              return false;
            }

          std::istringstream responseStringStream( inLine );
          
          timeval timeDelay;
          timeDelay.tv_sec = static_cast<long>(delay);
          timeDelay.tv_usec = 
            static_cast<long>((delay - static_cast<double>(timeDelay.tv_sec)) * 1000000.0);

          ResponseBase* response = 
            m_Simulator->getResponseFactory()->parse(commandName, timeDelay,
                                                     responseStringStream);
          
          if(response != 0)
            {
              responseMessageManager->addResponse(commandIndex, response);
            }
        }
      else
        {
          ;
        }
    }
  
  inputFile.close();

  return true;
}
