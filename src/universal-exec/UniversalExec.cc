/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "ExecApplication.hh"
#include "InterfaceManager.hh"
#include "InterfaceSchema.hh"
#include "TimeAdapter.hh"
#include "AdapterFactory.hh"
#include "Debug.hh"
#include "Node.hh"
#include "PlexilPlan.hh"
#include "lifecycle-utils.h"

#if HAVE_LUV_LISTENER
#include "LuvListener.hh"
#endif

#include <cstring>
#include <fstream>

using namespace PLEXIL;

int main_internal(int argc, char** argv)
{
  std::string planName("error");
  std::string debugConfig("Debug.cfg");
  std::string interfaceConfig("interface-config.xml");
  std::vector<std::string> libraryNames;
  std::vector<std::string> libraryPath;
  std::string
      usage(
          "Usage: universalExec -p <plan>\n\
                   [-l <library>]*\n\
                   [-L <library_directory>]*\n\
                   [-c <interface_config_file>]\n\
                   [-d <debug_config_file>]\n");
  bool luvRequest = false;

#if HAVE_LUV_LISTENER
  std::string luvHost = PLEXIL::LuvListener::LUV_DEFAULT_HOSTNAME();
  int luvPort = PLEXIL::LuvListener::LUV_DEFAULT_PORT();
  bool luvBlock = false;
  usage += "[-v [-h <luv_hostname>] [-n <luv_portnumber>] [-b] ]\n";
#endif

  // if not enough parameters, print usage
  if (argc < 2) {
    std::cout << usage << std::endl;
    return -1;
  }

  // parse out parameters
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-c") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      interfaceConfig = std::string(argv[i]);
	}
    else if (strcmp(argv[i], "-d") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      debugConfig = std::string(argv[i]);
	}
    else if (strcmp(argv[i], "-l") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n" 
				  << usage << std::endl;
		return -1;
	  }
      libraryNames.push_back(argv[i]);
	}
    else if (strcmp(argv[i], "-L") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      libraryPath.push_back(argv[i]);
	}
    else if (strcmp(argv[i], "-h") == 0) {
	  if (!luvRequest) {
		// interpret as simple help request
		std::cout << usage << std::endl;
		return 0;
	  }
#if HAVE_LUV_LISTENER
	  else if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      luvHost = argv[i];
#endif
	}
	else if (strcmp(argv[i], "-p") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      planName = argv[i];
	}
#if HAVE_LUV_LISTENER
    else if (strcmp(argv[i], "-v") == 0)
      luvRequest = true;
    else if (strcmp(argv[i], "-n") == 0) {
	  if (argc == (++i)) {
		std::cerr << "Error: Missing argument to the " << argv[i - 1] << " option.\n"
				  << usage << std::endl;
		return -1;
	  }
      std::istringstream buffer(argv[i]);
      buffer >> luvPort;
      SHOW(luvPort);
    } 
    else if (strcmp(argv[i], "-b") == 0)
      luvBlock = true;
#endif
    else {
      std::cerr << "Error: Unknown option '" << argv[i] << "'.\n" << usage << std::endl;
      return -1;
    }
  }
  // basic initialization

  std::ifstream dbgConfig(debugConfig.c_str());
  if (dbgConfig.good()) 
    readDebugConfigStream(dbgConfig);

  // get interface configuration file, if provided
  pugi::xml_document configDoc;
  if (!interfaceConfig.empty()) {
    std::cout << "Reading interface configuration from " << interfaceConfig << std::endl;
	pugi::xml_parse_result parseResult = configDoc.load_file(interfaceConfig.c_str());
    if (parseResult.status != pugi::status_ok) {
      std::cout << "WARNING: unable to load interface configuration file " 
                << interfaceConfig 
                << ":\n " 
                << parseResult.description()
                << "\nContinuing without interface configuration" << std::endl;
    }
	debugMsg("UniversalExec", " got configuration XML starting with " << configDoc.document_element().name());
  }

  // get Interfaces element
  pugi::xml_node configElt;
  if (configDoc.empty()) {
	// Construct default interface XML
	configElt = configDoc.append_child(PLEXIL::InterfaceSchema::INTERFACES_TAG());
	// Add a time adapter
	pugi::xml_node timeElt = configElt.append_child(PLEXIL::InterfaceSchema::ADAPTER_TAG());
	timeElt.append_attribute("AdapterType").set_value("OSNativeTime");
  }
  else {
	configElt = configDoc.child(PLEXIL::InterfaceSchema::INTERFACES_TAG());
	if (configElt.empty()) {
      std::cout << "ERROR: configuration XML lacks \"" << PLEXIL::InterfaceSchema::INTERFACES_TAG()
				<< "\" element; unable to initialize application"
                << std::endl;
      return -1;
	}
  }

#if HAVE_LUV_LISTENER
  // if a luv view is to be attached,
  // add dummy element for LuvListener
  if (luvRequest) {
	pugi::xml_document* luvConfig = 
	  PLEXIL::LuvListener::constructConfigurationXml(luvBlock,
													 luvHost.c_str(), 
													 luvPort);	
	// FIXME: add null check?
	configElt.append_copy(luvConfig->document_element());
	delete luvConfig;
  }
#endif

  // construct the application
  PLEXIL::ExecApplication _app;

  // initialize it
  std::cout << "Initializing application" << std::endl;
  if (!_app.initialize(configElt)) {
      std::cout << "ERROR: unable to initialize application"
                << std::endl;
      return -1;
    }

  // add library path
  if (!libraryPath.empty())
	_app.addLibraryPath(libraryPath);

  // start interfaces
  std::cout << "Starting interfaces" << std::endl;
  if (!_app.startInterfaces()) {
      std::cout << "ERROR: unable to start interfaces"
                << std::endl;
      return -1;
    }

  // start the application
  std::cout << "Starting the exec" << std::endl;
  if (!_app.run()) {
	std::cout << "ERROR: Failed to start Exec" << std::endl;
	return -1;
  }

  // Below this point, must be careful to shut down gracefully
  bool error = false;

  // if specified on command line, load Plexil libraries
  for (std::vector<std::string>::const_iterator libraryName = libraryNames.begin();
       libraryName != libraryNames.end();
       ++libraryName) {
	std::cout << "Loading library node from file '" << *libraryName << "'" << std::endl;
	pugi::xml_document libraryXml;
	pugi::xml_parse_result parseResult = libraryXml.load_file(libraryName->c_str());
    if (parseResult.status != pugi::status_ok) {
      std::cout << "XML error parsing library " << *libraryName
				<< " (offset " << parseResult.offset << "): "
                << parseResult.description() << std::endl;
	  error = true;
    }
	else if (!_app.addLibrary(&libraryXml)) {
      std::cout << "ERROR: unable to add library " << *libraryName << std::endl;
	  error = true;
    }
  }

  // load the plan
  if (!error && planName != "error") {
	pugi::xml_document plan;
	pugi::xml_parse_result parseResult = plan.load_file(planName.c_str());
    if (parseResult.status != pugi::status_ok) {
      std::cout << "Error parsing plan " << planName
				<< " (offset " << parseResult.offset << "): "
                << parseResult.description() << std::endl;
	  error = true;
    }
    else if (!_app.addPlan(&plan)) {
	  std::cout << "Unable to load plan '" << planName << "', exiting" << std::endl;
	  error = true;
	}
  }

  if (!error)
	_app.waitForPlanFinished();

  // clean up
  if (!_app.stop()) {
	std::cout << "ERROR: failed to stop Exec" << std::endl;
	return -1;
  }

  if (!_app.shutdown()) {
	std::cout << "ERROR: failed to shut down Exec" << std::endl;
	return -1;
  }

  std::cout << "Plan complete, Exec exited with"
            << (error ? " " : "out ") << "errors" << std::endl;
  return (error ? -1 : 0);
}

int main(int argc, char** argv)
{
  int result = main_internal(argc, argv);
  runFinalizers();
  return result;
}
