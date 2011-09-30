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

//
// This file contains a small example main() using the ExecApplication and related classes.
// It reads its interface configuration from an XML file
//


#include "Debug.hh"
#include "ExecApplication.hh"
#include "InterfaceSchema.hh"
#include "pugixml.hpp"
#include <fstream>
#include <iostream>
#include <vector>

void usage(const char* progname)
{
  std::cout << "Usage:\n "
            << progname << " [option]*\n"
            << "Supported options are:\n"
            << " -p <plan filename> (REQUIRED)\n"
            << " -c <config filename>\n"
            << " -d <debug-config filename>\n"
            << " -l <library filename> (multiple -l options are permitted)\n"
            << std::endl;
}

int main(int argc, char** argv)
{
  std::vector<const char*> libraryNames;
  const char* configFilename = NULL;
  const char* debugFilename = NULL;
  const char* planFilename = NULL;

  // if not enough parameters, print usage

  if (argc < 3)
    {
      usage(argv[0]);
      return -1;
    }

  // parse out parameters
   
  for (int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-c") == 0)
        {
          if (configFilename == NULL)
            {
              configFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple configuration files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else if (strcmp(argv[i], "-d") == 0)
        {
          if (debugFilename == NULL)
            {
              debugFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple debug files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else if (strcmp(argv[i], "-l") == 0)
        libraryNames.push_back(argv[++i]);
      else if (strcmp(argv[i], "-p") == 0)
        {
          if (planFilename == NULL)
            {
              planFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple plan files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else
        {
          std::cout << "ERROR: unknown option \"" 
                    << argv[i] 
                    << "\"\n" ;
          usage(argv[0]);
          return -1;
        }
    }

  // get debug file, if provided
  if (debugFilename != NULL)
    {
      std::ifstream debugConfig(debugFilename);
      if (debugConfig.good())
        {
          std::cout << "Reading debug configuration from "
                    << debugFilename
                    << std::endl;
          DebugMessage::readConfigFile(debugConfig);
        }
    }

  // get configuration file, if provided
  pugi::xml_document configDoc;
  if (configFilename != NULL)
    {
      std::cout << "Reading interface configuration from "
                << configFilename
                << std::endl;
	  pugi::xml_parse_result result = configDoc.load_file(configFilename);
      if (result.status != pugi::status_ok)
        {
          std::cout << "ERROR: unable to load configuration file "
                    << configFilename
                    << ":\n "
                    << result.description()
                    << std::endl;
          return -1;
        }
    }

  // get Interfaces element
  pugi::xml_node configElt;
  if (configDoc)
    configElt = configDoc.child(PLEXIL::InterfaceSchema::INTERFACES_TAG());

  // construct the application
  PLEXIL::ExecApplication app;

  // initialize it
  std::cout << "Initializing application" << std::endl;
  if (!app.initialize(configElt))
    {
      std::cout << "ERROR: unable to initialize application"
                << std::endl;
      return -1;
    }

  // start interfaces
  std::cout << "Starting interfaces" << std::endl;
  if (!app.startInterfaces())
    {
      std::cout << "ERROR: unable to start interfaces"
                << std::endl;
      return -1;
    }

  // start exec
  std::cout << "Starting exec" << std::endl;
  if (!app.run())
    {
      std::cout << "ERROR: unable to run exec"
                << std::endl;
      return -1;
    }

  // add libraries
  std::vector<const char*>::const_iterator libiter = libraryNames.begin();
  while (libiter != libraryNames.end())
    {
      const char* libFilename = *libiter;
      std::cout << "Reading library node from file "
                << libFilename
                << std::endl;
	  pugi::xml_document libDoc;
	  pugi::xml_parse_result result = libDoc.load_file(libFilename);
      if (result.status != pugi::status_ok)
        {
          std::cout << "ERROR: unable to load library XML from file "
                    << libFilename
                    << ":\n "
                    << result.description()
                    << std::endl;
          return -1;
        }
      if (!app.addLibrary(&libDoc))
        {
          std::cout << "ERROR: unable to add library "
                    << libFilename
                    << std::endl;
          return -1;
        }
      libiter++;
    }
  

  // add plan
  if (planFilename == NULL)
    {
      std::cout << "ERROR: no plan file specified\n";
      usage(argv[0]);
      return -1;
    }
  
  std::cout << "Reading plan from " << planFilename << std::endl;
  pugi::xml_document planDoc;
  pugi::xml_parse_result result = planDoc.load_file(planFilename);
  if (result.status != pugi::status_ok)
    {
      std::cout << "ERROR: unable to load plan XML from file "
                << planFilename
                << ":\n "
                << result.description()
                << std::endl;
      return -1;
    }

  std::cout << "Executing plan" << std::endl;
  if (!app.addPlan(&planDoc))
    {
      std::cout << "ERROR: unable to add plan "
                << planFilename
                << std::endl;
      return -1;
    }

  // wait til exec quiescent (NYI)
  app.waitForPlanFinished();
  std::cout << "Plan finished, stopping application" << std::endl;

  // stop exec
  if (!app.stop())
    {
      std::cout << "ERROR: unable to stop application"
                << std::endl;
      return -1;
    }

  // shut down exec
  std::cout << "Shutting down..." << std::flush;
  if (!app.shutdown())
    {
      std::cout << "ERROR: unable to shut down application"
                << std::endl;
      return -1;
    }

  std::cout << " shutdown complete, exiting." << std::endl;

  return 0;
}
