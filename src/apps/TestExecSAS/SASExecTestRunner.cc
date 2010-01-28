/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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
#include "SASAdapter.hh"
#include "AdapterFactory.hh"
#include "InterfaceSchema.hh"
#include "NewLuvListener.hh"
#include <fstream>

namespace PLEXIL 
{
  void SASExecTestRunner::usage() const
  {
    std::cout << "Usage: TestExecSAS [options] -p <plan>\n" 
	      << " where options are: \n"
	      << "  -l <library_file> -- library node file; multiple -l options permitted\n"
	      << "  -c <interface_config_file> -- use a custom interface configuration file\n"
	      << "  -d <debug_config_file> -- use a custom debug configuration file\n"
	      << "  -v [-h <hostname>] [-n <portnumber>] -b] -- use the LUV execution viewer;\n"

	      << "                     -h <hostname> -- LUV on remote host (default \"" << NewLuvListener::LUV_DEFAULT_HOSTNAME() << "\")\n"
	      << "                     -n <portnumber> -- IP port number for LUV (default " << NewLuvListener::LUV_DEFAULT_PORT() << ")\n"
	      << std::endl;
  }

  int SASExecTestRunner::run (int argc, char** argv)
  {
    std::string planName("error");
    std::string debugConfig("Debug.cfg");
    std::string interfaceConfig("interface-config.xml");
    std::vector<std::string> libraryNames;
    bool        luvRequest = false;
    std::string luvHost(NewLuvListener::LUV_DEFAULT_HOSTNAME());
    int         luvPort    = NewLuvListener::LUV_DEFAULT_PORT();
    bool        luvBlock   = false;

    // if not enough parameters, print usage

    if ( argc < 3)
      {
	usage();
	return -1;
      }
    // parse out parameters
   
    for (int i = 1; i < argc; ++i)
      {
	if (strcmp(argv[i], "-b") == 0)
	  luvBlock = true;
	else if (strcmp(argv[i], "-c") == 0)
	  interfaceConfig = std::string(argv[++i]);
	else if (strcmp(argv[i], "-d") == 0)
	  debugConfig = std::string(argv[++i]);
	else if (strcmp(argv[i], "-h") == 0)
	  luvHost = argv[++i];
	else if (strcmp(argv[i], "-l") == 0)
	  libraryNames.push_back(argv[++i]);
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
		      << "'.  \n" 
		      << std::endl;
	    usage();
	    return -1;
	  }
      }

    // Register factory for SASAdapter
    REGISTER_ADAPTER(SASAdapter, "StandAloneSimulator");

    std::cout << "Read plan: " << planName << std::endl;
    // basic initialization

    std::ifstream config(debugConfig.c_str());
    if (config.good())
      DebugMessage::readConfigFile(config);

    // get interface configuration file
    std::cout << "Reading interface configuration from "
	      << interfaceConfig
	      << std::endl;
    TiXmlDocument configDoc(interfaceConfig);
    if (!configDoc.LoadFile())
      {
	std::cout << "ERROR: unable to load configuration file "
		  << interfaceConfig
		  << ":\n "
		  << configDoc.ErrorDesc()
		  << std::endl;
	return -1;
      }

    // get Interfaces element
    TiXmlElement* configElt = 
      configDoc.FirstChildElement(PLEXIL::InterfaceSchema::INTERFACES_TAG());

    // Add dummy element for LuvListener
    if (luvRequest)
      {
	configElt->LinkEndChild(NewLuvListener::constructConfigurationXml(luvBlock,
									  luvHost.c_str(), 
									  luvPort));
      }

    // initialize application
    std::cout << "Initializing application" << std::endl;
    if (!execApplication.initialize(configElt))
      {
	std::cout << "ERROR: unable to initialize application"
		  << std::endl;
	return -1;
      }

    // start interfaces
    std::cout << "Starting interfaces" << std::endl;
    if (!execApplication.startInterfaces())
      {
	std::cout << "ERROR: unable to start interfaces"
		  << std::endl;
	return -1;
      }

    // start exec
    std::cout << "Starting exec" << std::endl;
    if (!execApplication.run())
      {
	std::cout << "ERROR: unable to run exec"
		  << std::endl;
	return -1;
      }

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

	if (!execApplication.addLibrary(&libraryXml))
	  {
	    std::cout << "ERROR: unable to add library "
		      << *libraryName
		      << std::endl;
	    return -1;
	  }
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
	std::cout << "Executing plan" << std::endl;
	if (!execApplication.addPlan(&plan))
	  {
	    std::cout << "ERROR: unable to add plan "
		      << planName
		      << std::endl;
	    return -1;
	  }
      }

    // wait til exec quiescent
    execApplication.waitForPlanFinished();
    std::cout << "Plan finished, stopping application" << std::endl;

    // stop exec
    if (!execApplication.stop())
      {
	std::cout << "ERROR: unable to stop application"
		  << std::endl;
	return -1;
      }

    // shut down exec
    std::cout << "Shutting down..." << std::flush;
    if (!execApplication.shutdown())
      {
	std::cout << "ERROR: unable to shut down application"
		  << std::endl;
	return -1;
      }

    std::cout << " shutdown complete, exiting." << std::endl;

    return 0;
  }

} // namespace
