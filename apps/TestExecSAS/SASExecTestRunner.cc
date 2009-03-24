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
#include "tinyxml.h"
#include "SocketException.h"
#include "CoreExpressions.hh"
#include "Expressions.hh"
#include "Debug.hh"
#include "StateManagerInit.hh"
#include "PlexilXmlParser.hh"
#include <time.h>
#include "Node.hh"
#include "PlexilPlan.hh"
#include "SASExecTestRunner.hh"
#include "SASAdaptor.hh"
#include <fstream>

namespace PLEXIL {

int SASExecTestRunner::run (int argc, char** argv, const ExecListener* listener)
{
   std::string scriptName("error");
   std::string planName("error");
   std::string debugConfig("Debug.cfg");
   std::vector<std::string> libraryNames;
   bool        luvRequest = false;
   std::string luvHost("Local");
   int         luvPort    = 9100;
   bool        luvBlock   = false;
   std::string usage(
      "Usage: exec-test-runner -s <script> -p <plan> [-l <library>]* [-d <debug_config_file>] [-v [-h <hostname>] [-n <portnumber>] -b];");

   // if not enough parameters, print usage

   if ( argc < 3)
   {
      std::cout << usage << std::endl;
      return -1;
   }
   // parse out parameters
   
   for (int i = 1; i < argc; ++i)
   {
      if (strcmp(argv[i], "-p") == 0)
         planName = argv[++i];
      else if (strcmp(argv[i], "-s") == 0)
         scriptName = argv[++i];
      else if (strcmp(argv[i], "-l") == 0)
         libraryNames.push_back(argv[++i]);
      else if (strcmp(argv[i], "-d") == 0)
        debugConfig = std::string(argv[++i]);
      else if (strcmp(argv[i], "-v") == 0)
         luvRequest = true;
      else if (strcmp(argv[i], "-b") == 0)
         luvBlock = true;
      else if (strcmp(argv[i], "-h") == 0)
         luvHost = argv[++i];
      else if (strcmp(argv[i], "-n") == 0)
      {
         std::stringstream buffer;
         buffer << argv[++i];
         buffer >> luvPort;
         SHOW(luvPort);
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


   std::cout << "Read plan: " << planName << std::endl;
   // basic initialization

   std::ifstream config(debugConfig.c_str());
   if (config.good()) DebugMessage::readConfigFile(config);

   initializeExpressions();
   initializeStateManagers();

   SASAdaptor sasAdaptor;
   threadedInterface.setDefaultInterface(sasAdaptor.getId());

   // create the exec

   PlexilExecId exec = (new PlexilExec())->getId();
   threadedInterface.setExec(exec);
   threadedInterface.resetQueue();

   // if specified on command line, load libraries

   for (std::vector<std::string>::const_iterator libraryName = libraryNames.begin();
              libraryName != libraryNames.end(); ++libraryName)
   {
      TiXmlDocument libraryXml(*libraryName);
      if (!libraryXml.LoadFile())
      {
         std::cout << "XML error parsing library '"
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
      PlexilNodeId libnode;
      try
	{
	  libnode = 
	    parser.parse(libraryXml.FirstChildElement("PlexilPlan")->FirstChildElement("Node"));
	}
      catch (ParserException& e)
	{
	  std::cout << "XML error parsing library '"
		    << *libraryName << "': \n"
		    << e.what()
		    << std::endl;
	  return -1;
	}
      exec->addLibraryNode(libnode);
   }

   if (planName != "error")
   {
      TiXmlDocument plan(planName);
      if (!plan.LoadFile())
      {
         std::cout << "XML error parsing plan '"
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
      PlexilNodeId root;
      try 
	{
	  root = 
	    parser.parse(plan.FirstChildElement("PlexilPlan") ->FirstChildElement("Node"));
	}
      catch (ParserException& e)
	{
	  std::cout << "XML error parsing plan '"
		    << planName << "': \n"
		    << e.what()
		    << std::endl;
	  return -1;
	}
      exec->addPlan(root);
   }

   std::cout << "Kicking off the exec." << std::endl;
   sleep(3);
   // kick off the exec.
   threadedInterface.run();

   
   // clean up

   delete (PlexilExec*) exec;
   return 0;
}

} // namespace
