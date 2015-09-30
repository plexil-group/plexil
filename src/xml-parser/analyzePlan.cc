/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "plexil-config.h"

#include "Debug.hh"
#include "Error.hh"
#include "Expressions.hh"
#include "Node.hh"
#include "lifecycle-utils.h"
#include "parsePlan.hh"
#include "planLibrary.hh"
#include "pugixml.hpp"
#include "test/TransitionExternalInterface.hh"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <cstring>

using PLEXIL::Node;
using PLEXIL::NodeType_error;
using PLEXIL::NodeType_NodeList;
using PLEXIL::NodeType_LibraryNodeCall;
using PLEXIL::nodeTypeString;
using PLEXIL::PlexilNodeType;

static unsigned int g_nodeCount = 0;

static unsigned int g_nodeTypeCounts[NodeType_error];

// Initialize to a semi-sane value
static std::vector<unsigned int> g_nodeChildCounts(16, 0);

typedef std::map<std::string, unsigned int> LibraryCallMap;
typedef std::pair<std::string, unsigned int> LibraryCallEntry;
LibraryCallMap g_calledLibs;

static unsigned int g_currentCallDepth = 0;
static unsigned int g_maxCallDepth = 0;

static void incrementNodeChildCount(unsigned int nKids)
{
  if (nKids >= g_nodeChildCounts.size())
    g_nodeChildCounts.resize(nKids * 2, 0); // grow rapidly to minimize resizing
  ++g_nodeChildCounts[nKids];
}

static void incrementLibraryCallCount(std::string const &name)
{
  LibraryCallMap::iterator it = g_calledLibs.find(name);
  if (it == g_calledLibs.end())
    g_calledLibs.insert(LibraryCallEntry(name, 1));
  else
    ++it->second;
}

static void initializeStatistics()
{
  g_nodeCount = 0;
  g_currentCallDepth = g_maxCallDepth = 0;
  for (size_t i = 0; i < NodeType_error; ++i)
    g_nodeTypeCounts[i] = 0;
  g_nodeChildCounts.clear();
  g_calledLibs.clear();
}

// Recursive function for plan traversal
static void getNodeStatistics(Node const *node)
{
  ++g_nodeCount;
  PlexilNodeType typ = node->getType();
  ++g_nodeTypeCounts[typ];
  std::vector<Node *> const &kids = node->getChildren();
  incrementNodeChildCount(kids.size());
  switch (typ) {
  case NodeType_LibraryNodeCall:
    incrementLibraryCallCount(kids.front()->getNodeId());
    ++g_currentCallDepth;
    if (g_currentCallDepth > g_maxCallDepth)
      g_maxCallDepth = g_currentCallDepth;
    getNodeStatistics(kids.front());
    --g_currentCallDepth;
    break;

  default:
  for (std::vector<Node *>::const_iterator it = kids.begin();
       it != kids.end();
       ++it)
    getNodeStatistics(*it);
    break;
  }
}

//
// Reporting
//

static void reportLibraryStatistics()
{
  if (!g_calledLibs.empty()) {
    std::cout << "--- Libraries ---\n\n";
    std::cout << g_calledLibs.size() << " libraries called:\n\n";
    for (LibraryCallMap::const_iterator it = g_calledLibs.begin();
         it != g_calledLibs.end();
         ++it)
      std::cout << it->first << " called " << it->second << " times\n";
    std::cout << '\n';
    std::cout << "Maximum library call depth: " << g_maxCallDepth << "\n\n";
  }
}

static void reportStatistics()
{
  std::cout << '\n';
  std::cout << g_nodeCount << " total nodes\n";
  std::cout << "\n--- Node Type Counts --- \n\n";
  for (size_t i = NodeType_NodeList; i < NodeType_error; ++i)
    if (g_nodeTypeCounts[i])
      std::cout << nodeTypeString((PlexilNodeType) i) << ": " << g_nodeTypeCounts[i] << '\n';
  std::cout << "\n--- Node Child Counts --- \n\n";
  std::cout << g_nodeChildCounts[0] << " leaf nodes\n";
  std::cout << g_nodeChildCounts[1] << " nodes with 1 child (includes library calls)\n";
  for (size_t i = 2; i < g_nodeChildCounts.size(); ++i)
    if (g_nodeChildCounts[i])
      std::cout << g_nodeChildCounts[i] << " nodes with " << i << " children\n";
  std::cout << '\n';

  reportLibraryStatistics();
}

static void cleanUpStatistics()
{
}

static void loadAndAnalyzePlan(std::string const &planFile)
{
  // Load the XML
  pugi::xml_document *doc = PLEXIL::loadXmlFile(planFile);
  checkParserException(doc, "File " << planFile << " not found");
  
  Node *root = PLEXIL::parsePlan(doc->document_element());
  checkParserException(root, "parsePlan returned NULL");

  // Analyze plan
  initializeStatistics();
  getNodeStatistics(root);
  reportStatistics();

  // Clean up
  cleanUpStatistics();
  delete root;
  delete doc;
}

static void usage()
{
  std::cout << "Usage: analyzePlan [options] <plan file>\n"
            << " Options:\n"
            << "  -L <dir>         Add <dir> to library path\n"
            << "  -h               Display this message and exit\n"
            << "  -d <debug file>  Use debug-file as debug message config (default Debug.cfg)\n"
            << std::endl;
}

extern "C"
int main(int argc, char *argv[])
{
  std::string debugConfig("Debug.cfg");
  std::string planFile;

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-d"))
      debugConfig = std::string(argv[++i]);
    else if (!strcmp(argv[i], "-h")) {
      usage();
      return 0;
    }
    else if (!strcmp(argv[i], "-L")) {
      PLEXIL::appendLibraryPath(argv[++i]);
    }
    else {
      if (!planFile.empty()) {
        std::cerr << "Multiple plan files specified" << std::endl;
        usage();
        return 1;
      }
      planFile = argv[i];
    }
  }

  if (planFile.empty()) {
    std::cerr << "No plan file specified" << std::endl;
    usage();
    return 1;
  }
  
  std::ifstream config(debugConfig.c_str());
  if (config.good()) {
     readDebugConfigStream(config);
     std::cout << "Reading configuration file " << debugConfig.c_str() << "\n";
  }
  
  std::cout << "Loading plan file " << planFile << " for analysis." << std::endl;

  try {
    // Initialize infrastructure
    Error::doThrowExceptions();
    PLEXIL::initializeExpressions();
    PLEXIL::TransitionExternalInterface intfc;
    PLEXIL::g_interface = &intfc;

    loadAndAnalyzePlan(planFile);

    PLEXIL::g_interface = NULL;
    runFinalizers();
  }
  catch (PLEXIL::ParserException const &e) {
    std::cerr << "Aborting due to parser exception:\n" << e.what() << std::endl;
    std::cout << "Aborted." << std::endl;
    return 1;
  }
  catch (Error const &e) {
    std::cerr << "Aborting due to error:\n" << e << std::endl;
    std::cout << "Aborted." << std::endl;
    return 1;
  }
  std::cout << "Done." << std::endl;
  return 0;
}
