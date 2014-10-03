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

#include "planLibrary.hh"

#include "Error.hh"
#include "lifecycle-utils.h"
#include "parsePlan.hh"
#include "PlexilSchema.hh"


#include <map>

using pugi::xml_document;
using pugi::xml_node;
using std::map;
using std::string;
using std::vector;

namespace PLEXIL
{
  //
  // Static variables local to this file
  //

  // List of library directories to search
  static vector<string> librarySearchPaths;

  // Place to store library nodes
  static map<string, xml_document *> libraryMap;

  vector<string> const &getLibraryPaths()
  {
    return librarySearchPaths;
  }

  void appendLibraryPath(string const &dirname)
  {
    librarySearchPaths.push_back(dirname);
  }

  void prependLibraryPath(string const &dirname)
  {
    librarySearchPaths.insert(librarySearchPaths.begin(), dirname);
  }

  void setLibraryPaths(std::vector<std::string> const &paths)
  {
    librarySearchPaths = paths;
  }

  static void cleanLibraryMap()
  {
    map<string, xml_document *>::iterator it = libraryMap.begin();
    while (it != libraryMap.end()) {
      xml_document *temp = it->second;
      libraryMap.erase(it);
      delete temp;
      it = libraryMap.begin();
    }
  }

  void addLibraryNode(string const &name, xml_document *doc)
    throw (ParserException)
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&cleanLibraryMap);
      sl_inited = true;
    }
    assertTrue_2(doc, "addLibraryNode: Null document");
    assertTrue_2(!name.empty(), "addLibraryNode: Empty name");
    // *** TODO: Check library is well formed ***
    libraryMap[name] = doc;
  }

  xml_document *loadLibraryFile(string const &filename)
  {
    // Check current working directory first
    xml_document *result = loadXmlFile(filename);
    if (result)
      return result;

    // Find the first occurrence of the library in this path
    vector<string>::const_iterator it = librarySearchPaths.begin();
    while (!result && it != librarySearchPaths.end()) {
      string candidateFile = *it + "/" + filename;
      result = loadXmlFile(candidateFile);
      if (result)
        return result;
      ++it;
    }
    return NULL;
  }

  // name could be node name, file name w/ or w/o directory, w/ w/o .plx
  xml_node loadLibraryNode(string const &name)
  {
    
    string nodeName = name;
    string fname = name;
    size_t pos = name.rfind(".plx");
    if (pos == string::npos)
      fname += ".plx";
    else
      nodeName = name.substr(0, pos);
    pos = nodeName.find_last_of("/\\");
    if (pos != string::npos)
      nodeName = nodeName.substr(++pos);
    // *** TEMP DEBUG ***
    std::cout << "loadLibraryNode(\"" << name << "\") - node " << nodeName << ", file " << fname << std::endl;

    xml_document *doc = loadLibraryFile(fname);
    if (!doc)
      return xml_node();
    xml_node theNode = doc->document_element().child(NODE_TAG);
    xml_node nodeIdXml = theNode.child(NODEID_TAG);
    checkParserExceptionWithLocation(nodeIdXml,
                                     doc->document_element(),
                                     "No " << NODEID_TAG << " element in library node, or not a PLEXIL plan");
    char const *nodeId = nodeIdXml.child_value();
    checkParserExceptionWithLocation(nodeName == nodeId,
                                     nodeIdXml,
                                     "loadLibraryNode: Requested " << nodeName
                                     << " but file contains " << nodeId);
    // *** TODO: handle global decls ***
    addLibraryNode(nodeName, doc);
    return theNode;
  }

  xml_node getLibraryNode(string const &name, bool loadIfNotFound)
  {
    map<string, xml_document *>::iterator it = libraryMap.find(name);
    if (it != libraryMap.end())
      return it->second->document_element().child(NODE_TAG);
    else if (loadIfNotFound)
      return loadLibraryNode(name);
    else
      return xml_node();
  }

} // namespace PLEXIL
