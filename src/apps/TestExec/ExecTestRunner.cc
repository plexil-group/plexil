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

#include "Logging.hh"
#include "PlexilExec.hh"
#include "ExecListener.hh"
#include "TestExternalInterface.hh"
#include "SocketException.h"
#include "CoreExpressions.hh"
#include "Expressions.hh"
#include "Debug.hh"
#include "StateManagerInit.hh"
#include "PlexilXmlParser.hh"
#include <time.h>
#include "Node.hh"
#include "PlexilPlan.hh"
#include "ExecTestRunner.hh"
#include "LuvListener.hh"
#include <fstream>
#include <string>

namespace PLEXIL {

int ExecTestRunner::run(int argc, char** argv, const ExecListener* listener) {
	std::string scriptName("error");
	std::string planName("error");
	std::string debugConfig("Debug.cfg");
	std::vector<std::string> libraryNames;
	bool luvRequest = false;
	std::string luvHost = LUV_DEFAULT_HOST;
	int luvPort = LUV_DEFAULT_PORT;
	bool luvBlock = false;
	std::string
			usage(
					"Usage: exec-test-runner -s <script> -p <plan> [-l <library>]* [-d <debug_config_file>] [-v [-h <hostname>] [-n <portnumber>] -b];");

	// if not enough parameters, print usage

	if (argc < 3) {
		warn(usage);
		return -1;
	}
	// parse out parameters

	for (int i = 1; i < argc; ++i) {
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
		else if (strcmp(argv[i], "-n") == 0) {
			std::stringstream buffer;
			buffer << argv[++i];
			buffer >> luvPort;
			SHOW(luvPort);
		} else if (strcmp(argv[i], "-log") == 0) {
			Logging::ENABLE_LOGGING = 1;
			Logging::set_log_file_name(argv[++i]);
		} else if (strcmp(argv[i], "-eprompt") == 0)
			Logging::ENABLE_E_PROMPT = 1;
		else if (strcmp(argv[i], "-wprompt") == 0)
			Logging::ENABLE_W_PROMPT = 1;
		else {
			warn("Unknown option '" << argv[i] << "'.  " << usage);
			return -1;
		}
	}

	if (Logging::ENABLE_LOGGING) {

#ifdef __linux__
		Logging::print_to_log(argv, argc);
#endif
#ifdef __APPLE__
		std::string cmd = "user command: ";
		for (int i = 1; i < argc; ++i)
		cmd = cmd + argv[i] + " ";

		Logging::print_to_log(cmd.c_str());
#endif
	}

	// if no script, error out

	if (scriptName == "error") {
		warn("No -s option found.  " << usage);
		return -1;
	}
	// basic initialization

	std::ifstream config(debugConfig.c_str());
	if (config.good())
		DebugMessage::readConfigFile(config);

	initializeExpressions();
	initializeStateManagers();

	// always preserve white space in XML
	TiXmlBase::SetCondenseWhiteSpace(false);

	// create the exec

	TestExternalInterface intf;
	PlexilExecId exec = (new PlexilExec())->getId();
	intf.setExec(exec);

	// if a luv view is to be attached

	if (luvRequest) {
		// create and add luv listener

		try {
			//          Socket* socket = new ClientSocket(luvHost, luvPort);
			//          ServerSocket* socket = new ServerSocket(luvPort);
			//          LuvListener* ll = new LuvListener(socket, luvBlock);
			//          LuvServer* luvServer = NULL;

			LuvListener* ll = new LuvListener(luvHost, luvPort, luvBlock);
			exec->addListener(ll->getId());
		} catch (SocketException se) {
			warn("WARNING: Unable to connect to Luv viewer: " << std::endl
					<< "  address: " << luvHost << ":" << luvPort << std::endl
					<< "   reason: " << se.description() << std::endl
					<< "Execution will continue without the viewer.");
		}
	}

	// if specified on command line, load libraries

	for (std::vector<std::string>::const_iterator libraryName =
			libraryNames.begin(); libraryName != libraryNames.end(); ++libraryName) {
		TiXmlDocument libraryXml(*libraryName);
		if (!libraryXml.LoadFile()) {
			checkParserException(false,
					"(line " << libraryXml.ErrorRow() << ", column " << libraryXml.ErrorCol()
					<< ") XML error parsing library '" << *libraryName << "': " << libraryXml.ErrorDesc());
			return -1;
		}

		PlexilXmlParser parser;
		PlexilNodeId libnode;
		try {
			libnode
					= parser.parse(
							libraryXml.FirstChildElement("PlexilPlan")->FirstChildElement(
									"Node"));
		} catch (ParserException& e) {
			checkParserException(false,
					"XML error parsing library '" << *libraryName << "': \n" << e.what());
			return -1;
		}
		exec->addLibraryNode(libnode);
	}

	if (planName != "error") {
		TiXmlDocument plan(planName);
		if (!plan.LoadFile()) {
			checkParserException(false,
					"(line " << plan.ErrorRow() << ", column " << plan.ErrorCol()
					<< ") XML error parsing plan '" << planName << "': " << plan.ErrorDesc());
			return -1;
		}

		PlexilXmlParser parser;
		PlexilNodeId root;
		try {
			root = parser.parse(
					plan.FirstChildElement("PlexilPlan") ->FirstChildElement("Node"));
		} catch (ParserException& e) {
			return -1;
		}
		if (!exec->addPlan(root)) {
		  warn("Adding plan " << planName << " failed");
		  return -1;
		}
	}

	// Add listener if specified
	if (listener)
		((PlexilExec*) exec)->addListener(listener->getId());

	// load script

	TiXmlDocument script(scriptName);
	if (!script.LoadFile()) {
		checkParserException(false,
				"(line " << script.ErrorRow() << ", column " << script.ErrorCol()
				<< ") XML error parsing script '" << scriptName << "': " << script.ErrorDesc());
		return -1;
	}
	// execute plan

	clock_t time = clock();
	TiXmlElement* scriptElement = script.FirstChildElement("PLEXILScript");
	if (scriptElement == 0) {
		warn("File '" << scriptName << "' is not a valid PLEXIL simulator script");
		return -1;
	}
	intf.run(*scriptElement);
	debugMsg("Time", "Time spent in execution: " << clock() - time);

	// clean up

	delete (PlexilExec*) exec;
	return 0;
}

} // namespace
