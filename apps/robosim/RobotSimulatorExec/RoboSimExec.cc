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

#include "PlexilExec.hh"
#include "ThreadedExternalInterface.hh"
#include "tinyxml.h"
#include "CoreExpressions.hh"
#include "Expressions.hh"
#include "Debug.hh"
#include "StateManagerInit.hh"
#include "PlexilXmlParser.hh"
#include "Node.hh"
#include "PlexilPlan.hh"
#include "RoboSimInterfaceAdaptor.hh"
#include "LuvListener.hh"


int main (int argc, char** argv)
{
   std::string planName("error");
   std::string debugConfig("Debug.cfg");
   std::string ipAddress("127.0.0.1");
   int portNumber=6164;
   bool        luvRequest = false;
   std::string luvHost    = LUV_DEFAULT_HOST;
   int         luvPort    = LUV_DEFAULT_PORT;
   bool        luvBlock   = false;
   std::string usage(
      "Usage: roboSimExec -p <plan> [-d <debug_config_file>] [-i <ip_address_comm_server>] [-p <port_number_comm_server>] [-v [-h <hostname>] [-n <portnumber>] -b];");

   // if not enough parameters, print usage

   if (argc < 2)
   {
      std::cout << usage << std::endl;
      return -1;
   }
   // parse out parameters
   
   for (int i = 1; i < argc; ++i)
   {
      if (strcmp(argv[i], "-p") == 0)
         planName = argv[++i];
      else if (strcmp(argv[i], "-d") == 0)
        debugConfig = std::string(argv[++i]);
      else if (strcmp(argv[i], "-i") == 0)
        ipAddress = std::string(argv[++i]);
      else if (strcmp(argv[i], "-p") == 0)
        portNumber = atoi(argv[++i]);
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
   // basic initialization

   std::ifstream config(debugConfig.c_str());
   if (config.good()) DebugMessage::readConfigFile(config);

   PLEXIL::initializeExpressions();
   PLEXIL::initializeStateManagers();

   // create the exec

   PLEXIL::PlexilExecId exec = (new PLEXIL::PlexilExec())->getId();
   PLEXIL::ThreadedExternalInterface _plxl_interface;
   _plxl_interface.setExec(exec);
   // Clear interface queue before loading plan
   _plxl_interface.resetQueue();
   
   RoboSimInterfaceAdaptor* _plxl_adaptor = 
     new RoboSimInterfaceAdaptor("RoboSimExec", ipAddress, portNumber);

   _plxl_interface.setDefaultInterface(_plxl_adaptor->getId());

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
      PLEXIL::PlexilXmlParser parser;
      PLEXIL::PlexilNodeId root =
         parser.parse(plan.FirstChildElement("PlexilPlan")
                      ->FirstChildElement("Node"));
      exec->addPlan(root);
   }


   // if a luv view is to be attached

   if (luvRequest)
   {
      // create and add luv listener

      try
      {
        PLEXIL::LuvListener* ll = new PLEXIL::LuvListener(luvHost, luvPort, luvBlock);
        exec->addListener(ll->getId());
      }
      catch (SocketException se)
      {
         std::cout 
            << "WARNING: Unable to connect to Luv viewer: "  << std::endl
            << "  address: " << luvHost << ":" << luvPort    << std::endl
            << "   reason: " << se.description()             << std::endl
            << "Execution will continue without the viewer." << std::endl 
            << std::endl;
      }
   }

   // execute plan
   std::cout << "Stepping the exec" << std::endl;
   exec->step();
   _plxl_interface.run();

   // clean up

   delete (PLEXIL::PlexilExec*) exec;
   delete _plxl_adaptor;
   return 0;
}
