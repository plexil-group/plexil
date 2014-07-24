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

#ifndef _H_PlexilXmlParser
#define _H_PlexilXmlParser

#include "ConstantMacros.hh"
#include "ParserException.hh"
#include "pugixml.hpp"

#include <vector>

namespace PLEXIL
{
  // Forward references
  class PlexilNode;

  /*
   * @brief A stateless class for parsing Plexil XML plans.
   */
  class PlexilXmlParser
  {
  public:
    //
    // Constants
    //

    // Ensure text consisting only of whitespace is preserved.
    DECLARE_STATIC_CLASS_CONST(unsigned int, PUGI_PARSE_OPTIONS, pugi::parse_default | pugi::parse_ws_pcdata_single);

    /*
     * @brief Load the named library node from a file on the given path.
     * @param name Name of the node.
     * @param path Vector of places to search for the file.
     * @return The loaded node, or noId() if not found or error.
     */
    static PlexilNode *findLibraryNode(const std::string& name,
                                       const std::vector<std::string>& path);

    /*
     * @brief Load the named plan from a file on the given path.
     * @param name Name of the node.
     * @param fileName Name of the file.
     * @param path Vector of places to search for the file.
     * @return The loaded node, or noId() if not found or error.
     */
    static PlexilNode *findPlan(const std::string& name,
                                const std::string& fileName,
                                const std::vector<std::string>& path);

    /*
     * @brief Load the named plan from a file in the given directory.
     * @param name Name of the desired node.
     * @param filename Candidate file for this node.
     * @return The loaded node, or noId() if not found or error.
     */
    static PlexilNode *loadPlanNamed(const std::string& name,
                                     const std::string& filename)
      throw(ParserException);

    // Deprecated.
    static PlexilNode *parse(const std::string& str, bool isFile)
      throw(ParserException);

    static PlexilNode *parse(const char* text)
    throw(ParserException);

    // Presumes XML is a PlexilPlan or Node element.
    static PlexilNode *parse(pugi::xml_node xml)
      throw(ParserException);

  private:

    // Explicitly not implemented
    PlexilXmlParser();
    PlexilXmlParser(const PlexilXmlParser&);
    PlexilXmlParser& operator=(const PlexilXmlParser&);
    ~PlexilXmlParser();
  };
}

#endif

