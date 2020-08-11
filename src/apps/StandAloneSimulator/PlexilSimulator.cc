/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "Agenda.hh"
#include "IpcCommRelay.hh"
#include "Simulator.hh"
#include "SimulatorScriptReader.hh"

#include "Debug.hh"

#include <fstream>

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

static void usage(std::ostream &stream = std::cout)
{
  stream << "Usage: simulator <script file>* [options ...]\n"
         << " Options are:\n"
         << "  -n <agent name>                (default is \"RobotYellow\")\n"
         << "  -t <telemetry script file>\n"
         << "  -central <host>:<port>         (default is localhost:1381)\n"
         << "  -d <debug config file>         (default is SimDebug.cfg)\n"
         << std::endl;
}

int main(int argc, char** argv)
{
  // defaults for command line args
  std::vector<std::string> scriptNames;
  std::string agentName("RobotYellow");
  std::string telemetryScriptName("");
  std::string centralhost("localhost:1381");
  std::string debugConfig("SimDebug.cfg");

  //
  // Parse command arguments
  //

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      // It's an option
      if (strcmp(argv[i], "-h") == 0) {
        usage();
        return 0;
      }
      else if (strcmp(argv[i], "-d") == 0)
        debugConfig = argv[++i];
      else if (strcmp(argv[i], "-central") == 0)
        centralhost = argv[++i];
      else if (strcmp(argv[i], "-n") == 0)
        agentName = argv[++i];
      else if (strcmp(argv[i], "-t") == 0) {
        telemetryScriptName = argv[++i];
        std::cout << "WARNING: The '-t' option is deprecated.\n\
Telemetry scripts can be converted to the unified format by adding the line:\n\n\
BEGIN_TELEMETRY\n\n\
at the top of the script."
                  << std::endl;
      }
      else {
        std::cerr << "Unknown option '" 
                  << argv[i] 
                  << "'." 
                  << std::endl;
        usage(std::cerr);
        return 1;
      }
    }
    else {
      // presume it's a file name
      scriptNames.push_back(argv[i]);
    }
  }

  if (scriptNames.empty() && telemetryScriptName.empty()) {
    std::cerr << "Error: no script(s) supplied" << std::endl;
    usage(std::cerr);
    return 1;
  }

  if (!debugConfig.empty()) {
    std::ifstream dc(debugConfig.c_str());
    if (dc.fail()) {
      std::cerr << "Warning: unable to open debug configuration file "
                << debugConfig << std::endl;
    }
    else {
      PLEXIL::setDebugOutputStream(std::cerr);
      if (!PLEXIL::readDebugConfigStream(dc)) {
        std::cerr << "Error in debug configuration file " << debugConfig << std::endl;
        return 1;
      }
    }
  }


  //
  // Read the scripts
  //

  ResponseManagerMap *mgrMap = new ResponseManagerMap();
  Agenda *agenda = makeAgenda();
  {
    // The script reader can go away as soon as we finish reading scripts.
    SimulatorScriptReader *rdr = makeScriptReader(mgrMap, agenda);
    for (std::vector<std::string>::const_iterator it = scriptNames.begin();
         it != scriptNames.end();
         it++) {
      debugMsg("PlexilSimulator", " reading script " << *it);
      rdr->readScript(*it);
    }
    if (!telemetryScriptName.empty()) {
      debugMsg("PlexilSimulator",  
               " reading telemetry script " << telemetryScriptName);
      rdr->readScript(telemetryScriptName, true);
    }

    delete rdr;
  }

  //
  // Run the simulator
  //
  
  // Comm Relay has to be destroyed before we can nuke the simulator
  IpcCommRelay* plexilRelay = new IpcCommRelay(agentName, centralhost);

  // Simulator instance is responsible for deleting map, agenda
  Simulator *mySimulator = makeSimulator(plexilRelay, mgrMap, agenda);

  // Run until interrupted
  mySimulator->simulatorTopLevel();

  delete plexilRelay;
  delete mySimulator;
  return 0;
}
