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

#include "PlexilExec.hh"
#include "TestExternalInterface.hh"
#include "tinyxml.h"
#include "CoreExpressions.hh"
#include "Expressions.hh"
#include "Debug.hh"
#include "StateManagerInit.hh"
#include "PlexilXmlParser.hh"

// CORBA stuff
#include "CorbaHelper.hh"
#include "EventChannelExecListener.hh"
#include "StructuredEventFormatter.hh"

#include <time.h>

#include "Node.hh"
#include "PlexilPlan.hh"

using namespace PLEXIL;

int main(int argc, char** argv)
{
  std::string scriptName("error");
  std::string planName("error");
  std::string eventChannelName("error");
  std::string usage("Usage:\n\
 event-channel-test-runner -e <event_channel> -s <script> -p <plan> [-l <library>]*\n\
                           -ORBInitRef NameService=corbaloc:iiop:<hostname>:<port#>/NameService");
  std::vector<std::string> libraryNames;

  // if not enough parameters, print usage

  if (argc < 7 && argc != 3)
    {
      std::cout << usage << std::endl;
      return -1;
    }
  // parse out parameters

  for (int i = 1; i < argc; i += 2)
    {
      if (strcmp(argv[i], "-e") == 0)
	eventChannelName = argv[i+1];
      else if (strcmp(argv[i], "-p") == 0)
	planName = argv[i+1];
      else if (strcmp(argv[i], "-s") == 0)
	scriptName = argv[i+1];
      else if (strcmp(argv[i], "-l") == 0)
	libraryNames.push_back(argv[i+1]);
      else if (strcmp(argv[i], "-ORBInitRef") == 0)
	{
	  // ignore, pass to TAO later
	}
      else
	{
	  std::cout << "Unknown option '" 
		    << argv[i] 
		    << "'.  " 
		    << usage 
		    << std::endl;
	  return -1;
	}
    }
  // if no script, error out

  if (scriptName == "error")
    {
      std::cout << "No -s option found.  " << usage << std::endl;
      return -1;
    }
  //   if(planName == "error") 
  //   {
  //     std::cout << "No -p option found.  " << usage << std::endl;
  //     return -1;
  //   }

  // basic initialization

  initializeExpressions();
  initializeStateManagers();

  // if specified on command line, load libraries

  std::vector<PlexilNodeId> libraries;
  for (std::vector<std::string>::const_iterator libraryName = libraryNames.begin();
       libraryName != libraryNames.end(); ++libraryName)
    {
      TiXmlDocument libraryXml(*libraryName);
      if (!libraryXml.LoadFile())
	{
	  std::cout << "Error parsing library '"
		    << *libraryName << "': "
		    << libraryXml.ErrorDesc()
		    << " line "
		    << libraryXml.ErrorRow()
		    << " column "
		    << libraryXml.ErrorCol()
		    << std::endl;
	  return -1;
	}

      PlexilXmlParser parser;
      libraries.push_back(
			  parser.parse(libraryXml.FirstChildElement("PlexilPlan")
				       ->FirstChildElement("Node")));
    }
  // load plan

  TestExternalInterface intf;
  PlexilExecId exec;
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
      PlexilXmlParser parser;
      PlexilNodeId root =
	parser.parse(plan.FirstChildElement("PlexilPlan")
		     ->FirstChildElement("Node"));
      root->link(libraries);
      exec = (new PlexilExec(root))->getId();

      // *** debug only ***
      std::cout << "Plan " << planName << " loaded" << std::endl;
    }
  else
    exec = (new PlexilExec())->getId();

  // activate CORBA
  CorbaHelper & helper = CorbaHelper::getInstance();
  if (!helper.initializeOrb(argc, argv))
    {
      std::cerr << "Unable to initialize CORBA ORB. Exiting." << std::endl;
      return -1;
    }

  if (!helper.initializePOA())
    {
      std::cerr << "Unable to initialize CORBA POA. Exiting." << std::endl;
      return -1;
    }
  
  if (!helper.initializeNameService())
    {
      std::cerr << "Unable to access CORBA name service. Exiting." << std::endl;
      return -1;
    }

  // Add listener
  EventChannelExecListener listener;
  if (listener.connect(eventChannelName))
    {
      exec->addListener(listener.getId());
      listener.setFormatter((new StructuredEventFormatter("StructuredEventTest"))->getId());
    }
  else
    {
      std::cerr << "Couldn't connect to event channel, not using exec listener" << std::endl;
    }

  // load script

  ExternalInterface::instance()->setExec(exec);
  TiXmlDocument script(scriptName);
  if (!script.LoadFile())
    {
      std::cout << "Error parsing script '" 
                << scriptName 
                << "': " 
                << script.ErrorDesc() 
                << " line " 
                << script.ErrorRow() 
                << " column " 
                << script.ErrorCol() 
                << std::endl;
      return -1;
    }
  std::cout << "Script " << scriptName << " loaded" << std::endl;

  // execute plan

  clock_t time = clock();
  std::cout << "Running script" << std::endl;
  intf.run(*(script.FirstChildElement("PLEXILScript")));
  std::cout << "Script completed" << std::endl;
  debugMsg("Time", "Time spent in execution: " << clock() - time);

  // clean up

  delete (PlexilExec*) exec;
  return 0;
}
