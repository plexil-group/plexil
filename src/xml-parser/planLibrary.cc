/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "planLibrary.hh"

#include "Error.hh"
#include "lifecycle-utils.h"
#include "map-utils.hh"
#include "parsePlan.hh"
#include "PlexilSchema.hh"
#include "SimpleMap.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

using pugi::xml_document;
using pugi::xml_node;
using std::string;
using std::vector;

namespace PLEXIL
{
  //
  // Static variables local to this file
  //

  // List of library directories to search
  static vector<string> s_librarySearchPaths;

  // Place to store library nodes and their global contexts
  typedef SimpleMap<string, Library> LibraryMap;
  static LibraryMap s_libraryMap;

  vector<string> const &getLibraryPaths()
  {
    return s_librarySearchPaths;
  }

  void appendLibraryPath(string const &dirname)
  {
    s_librarySearchPaths.push_back(dirname);
  }

  void prependLibraryPath(string const &dirname)
  {
    s_librarySearchPaths.insert(s_librarySearchPaths.begin(), dirname);
  }

  void setLibraryPaths(std::vector<std::string> const &paths)
  {
    s_librarySearchPaths = paths;
  }

  // Call at exit
  static void cleanLibraryMap()
  {
    for (LibraryMap::iterator it = s_libraryMap.begin(); it != s_libraryMap.end(); ++it) {
      Library &l = it->second;
      delete l.doc;
      l.doc = NULL;
      delete l.symtab;
      l.symtab = NULL;
    }
    s_libraryMap.clear();
  }

  // Internal function
  static xml_document *loadLibraryFile(string const &filename)
  {
    // Check current working directory first
    xml_document *result = loadXmlFile(filename);
    if (result)
      return result;

    // Find the first occurrence of the library in this path
    vector<string>::const_iterator it = s_librarySearchPaths.begin();
    while (!result && it != s_librarySearchPaths.end()) {
      string candidateFile = *it + "/" + filename;
      result = loadXmlFile(candidateFile);
      if (result)
        return result;
      ++it;
    }
    return NULL;
  }

  // name could be node name, file name w/ or w/o directory, w/ w/o .plx
  Library const *loadLibraryNode(char const *name)
  {
    string nodeName = name;
    string fname = name;
    size_t pos = fname.rfind(".plx");
    if (pos == string::npos)
      fname += ".plx";
    else
      nodeName = nodeName.substr(0, pos);
    pos = nodeName.find_last_of("/\\");
    if (pos != string::npos)
      nodeName = nodeName.substr(++pos);

    xml_document *doc = loadLibraryFile(fname);
    if (!doc)
      return NULL;
    
    // Check whether document actually contains the named plan
    char const *nodeId = doc->document_element().child(NODE_TAG).child_value(NODEID_TAG);
    if (nodeName != nodeId) {
      warn("Unable to load library node \"" << nodeName
           << "\": file " << fname << " does not contain " << nodeId);
      delete doc;
      return NULL;
    }

    return loadLibraryDocument(doc);
  }

  // Internal fn
  static Library *findLibraryNode(char const *name)
  {
    LibraryMap::iterator it = s_libraryMap.find<char const *, CStringComparator>(name);
    if (it != s_libraryMap.end())
      return &it->second;
    else
      return NULL;
  }

  Library const *loadLibraryDocument(xml_document *doc)
  {
    // Check if already loaded
    xml_node const plan = doc->document_element();
    char const *nodeId = plan.child(NODE_TAG).child_value(NODEID_TAG);
    Library *l = findLibraryNode(nodeId);

    if (l && plan == l->doc->document_element()) {
      // Same plan, no need to go any further
      delete doc;
      return l;
    }

    SymbolTable *symtab = NULL;
    try {
      symtab = checkPlan(plan);
    }
    catch (ParserException const &exc) {
      delete doc;
      warn("Unable to load library node \"" << nodeId << "\": "
           << exc.what());
      return NULL;
    }
    catch (...) {
      delete doc;
      throw;
    }

    // Success!
    if (l) {
      // Replace previous version
      delete l->doc;
      delete l->symtab;
      l->doc = doc;
      l->symtab = symtab;
      return l;
    }
    else {
      // If this is first library added, set up the cleanup function
      static bool sl_inited = false;
      if (!sl_inited) {
        plexilAddFinalizer(&cleanLibraryMap);
        sl_inited = true;
      }
      
      std::string nodeStr = nodeId;
      s_libraryMap[nodeStr] = Library(doc, symtab);
      return &s_libraryMap[nodeStr];
    }
  }

  bool isLibraryLoaded(char const *name)
  {
    LibraryMap::iterator it = s_libraryMap.find<char const *, CStringComparator>(name);
    return it != s_libraryMap.end();
  }

  Library const *getLibraryNode(char const *name, bool loadIfNotFound)
  {
    LibraryMap::iterator it = s_libraryMap.find<char const *, CStringComparator>(name);
    if (it != s_libraryMap.end())
      return &it->second;
    else if (loadIfNotFound)
      return loadLibraryNode(name);
    else
      return NULL;
  }

} // namespace PLEXIL
