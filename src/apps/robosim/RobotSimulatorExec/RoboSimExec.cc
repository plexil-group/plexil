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

#include <fstream>
#include <time.h>

//#include "PlexilExec.hh"
#include "ExecApplication.hh"
#include "InterfaceManager.hh"
#include "InterfaceSchema.hh"
#include "TimeAdapter.hh"
#include "Debug.hh"
#include "PlexilXmlParser.hh"
#include "Node.hh"
#include "PlexilPlan.hh"
#include "RoboSimInterfaceAdapter.hh"
#include "NewLuvListener.hh"
#include "SocketException.h"


int main (int argc, char** argv)
{
  std::string planName("error");
  std::string debugConfig("Debug.cfg");
  std::string interfaceConfig("");
  std::string ipAddress("127.0.0.1");
  int portNumber=6164;
  bool        luvRequest = false;
  std::string luvHost    = PLEXIL::NewLuvListener::LUV_DEFAULT_HOSTNAME();
  int         luvPort    = PLEXIL::NewLuvListener::LUV_DEFAULT_PORT();
  bool        luvBlock   = false;
  std::string usage(
		    "Usage: roboSimExec -p <plan>\n\
                   [-c <interface_config_file>]\n\
                   [-d <debug_config_file>]\n\
                   [-i <ip_address_comm_server>]\n\
                   [-cp <port_number_comm_server>]\n\
                   [-v [-h <luv_hostname>] [-n <luv_portnumber>] -b]");

  // if not enough parameters, print usage

  if (argc < 2)
    {
      std::cout << usage << std::endl;
      return -1;
    }
  // parse out parameters
   
  for (int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-b") == 0)
	luvBlock = true;
      else if (strcmp(argv[i], "-c") == 0)
        interfaceConfig = std::string(argv[++i]);
      else if (strcmp(argv[i], "-cp") == 0)
        portNumber = atoi(argv[++i]);
      else if (strcmp(argv[i], "-d") == 0)
        debugConfig = std::string(argv[++i]);
      else if (strcmp(argv[i], "-i") == 0)
        ipAddress = std::string(argv[++i]);
      else if (strcmp(argv[i], "-h") == 0)
	luvHost = argv[++i];
      else if (strcmp(argv[i], "-n") == 0)
	{
	  std::stringstream buffer;
	  buffer << argv[++i];
	  buffer >> luvPort;
	  SHOW(luvPort);
	}
      else if (strcmp(argv[i], "-p") == 0)
	planName = argv[++i];
      else if (strcmp(argv[i], "-v") == 0)
	luvRequest = true;
      else
	{
	  std::cout << "Unknown option '" 
		    << argv[i] 
		    << "'.\n" 
		    << usage 
		    << std::endl;
	  return -1;
	}
    }
  // basic initialization

  std::ifstream dbgConfig(debugConfig.c_str());
  if (dbgConfig.good()) 
    DebugMessage::readConfigFile(dbgConfig);

  // get interface configuration file, if provided
  TiXmlDocument* configDoc = NULL;
  if (!interfaceConfig.empty())
    {
      std::cout << "Reading interface configuration from "
                << interfaceConfig
                << std::endl;
      configDoc = new TiXmlDocument(interfaceConfig);
      if (!configDoc->LoadFile())
        {
          std::cout << "ERROR: unable to load interface configuration file "
                    << interfaceConfig
                    << ":\n "
                    << configDoc->ErrorDesc()
                    << std::endl;
          return -1;
        }
    }

  // get Interfaces element
  TiXmlElement* configElt = NULL;
  if (configDoc == NULL)
    {
      configElt = new TiXmlElement(PLEXIL::InterfaceSchema::INTERFACES_TAG());
      // Add a time adapter
      TiXmlElement* timeElt = new TiXmlElement(PLEXIL::InterfaceSchema::ADAPTER_TAG());
      timeElt->SetAttribute("AdapterType", "OSNativeTime");
      configElt->LinkEndChild(timeElt);
    }
  else
    {
      configElt = configDoc->FirstChildElement(PLEXIL::InterfaceSchema::INTERFACES_TAG());
    }

  // if a luv view is to be attached,
  // add dummy element for LuvListener
  if (luvRequest)
    {
      configElt->LinkEndChild(PLEXIL::NewLuvListener::constructConfigurationXml(luvBlock,
										luvHost.c_str(), 
										luvPort));
    }

  // construct the application
  PLEXIL::ExecApplication _app;

  // initialize it
  std::cout << "Initializing application" << std::endl;
  if (!_app.initialize(configElt))
    {
      std::cout << "ERROR: unable to initialize application"
                << std::endl;
      return -1;
    }

  // Add RoboSim adapter
  RoboSimInterfaceAdapter* _plxl_adapter = 
    new RoboSimInterfaceAdapter(_app.getInterfaceManagerReference(),
				"RoboSimExec",
				ipAddress, 
				portNumber);
  _plxl_adapter->initialize();
  _app.getInterfaceManager()->setDefaultInterface(_plxl_adapter->getId());

  // start interfaces
  std::cout << "Starting interfaces" << std::endl;
  if (!_app.startInterfaces())
    {
      std::cout << "ERROR: unable to start interfaces"
                << std::endl;
      return -1;
    }

  // execute plan
  std::cout << "Starting the exec" << std::endl;
  _app.run();

  if (planName != "error")
    {
      TiXmlDocument plan(planName);
      if (!plan.LoadFile())
	{
	  std::cout << "Error parsing plan '"
		    << planName << "': " 
		    << plan.ErrorDesc()
		    << " line "
		    << plan.ErrorRow() 
		    << " column " 
		    << plan.ErrorCol()
		    << std::endl;
	  return -1;
	}
      _app.addPlan(&plan);
    }

  _app.waitForPlanFinished();

  // clean up

  return 0;
}




