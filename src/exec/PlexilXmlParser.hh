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

#include "PlexilPlan.hh"
#include "ConstantMacros.hh"
#include "ParserException.hh"
#include "pugixml.hpp"
#include <map>

namespace PLEXIL
{

  template <typename Ret>
  class PlexilElementParser
  {
  public:
    PlexilElementParser() {}
    virtual ~PlexilElementParser() {}

    virtual Ret *parse(const pugi::xml_node& xml)
      throw(ParserException) = 0;

  private:
    // deliberately not implemented
    PlexilElementParser(const PlexilElementParser&);
    PlexilElementParser& operator=(const PlexilElementParser&);
  };

  typedef PlexilElementParser<PlexilNodeBody> PlexilBodyParser;
  typedef PlexilElementParser<PlexilExpr> PlexilExprParser;

  /*
   * @brief A stateless class for parsing Plexil XML plans.
   */
  class PlexilXmlParser : public PlexilParser
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

    // These don't really need to be public,
    // but the alternative is forward declaring the classes of their callers
    // so that they can be declared friends.
    // C++ sucks at encapsulation.
    // -- Chucko 12 Nov 2010

    //this is used to get around the old way of handling node references
    static PlexilNodeRef *getNodeRef(const pugi::xml_node& ref, 
                                      const pugi::xml_node& node)
      throw(ParserException);
    static pugi::xml_node getNodeParent(const pugi::xml_node& node);

    static PlexilExpr *parseExpr(const pugi::xml_node& xml)
      throw(ParserException);
    static PlexilNode *parseNode(const pugi::xml_node& node)
      throw(ParserException);
    static PlexilNodeRef *parseNodeRef(const pugi::xml_node& xml)
      throw(ParserException);
    static PlexilState *parseState(const pugi::xml_node& xml)
      throw(ParserException);
    static std::vector<PlexilResource *> parseResource(const pugi::xml_node& xml)
      throw(ParserException);

    static void deleteParsers();

  private:

    // Explicitly not implemented
    PlexilXmlParser();
    PlexilXmlParser(const PlexilXmlParser&);
    PlexilXmlParser& operator=(const PlexilXmlParser&);
    ~PlexilXmlParser();

    static PlexilInterface *parseDeprecatedInterface(const pugi::xml_node& intf)
      throw(ParserException);
    static PlexilInterface *parseInterface(const pugi::xml_node& intf)
      throw(ParserException);
    static void parseInOrInOut(const pugi::xml_node& inOrInOut, 
                               PlexilInterface *interface,
                               bool isInOut)
      throw(ParserException);
    static void parseDeclarations(const pugi::xml_node& decls, PlexilNode *node)
      throw(ParserException);
    static PlexilVar* parseDeclaration(const pugi::xml_node& decl)
      throw(ParserException);
    static PlexilVar* parseArrayDeclaration(const pugi::xml_node& decls)
      throw(ParserException);
    static PlexilVar* parseAtomicOrStringDeclaration(const pugi::xml_node& decls)
      throw(ParserException);
    static PlexilNodeBody *parseBody(const pugi::xml_node& body)
      throw(ParserException);

    static PlexilNodeRef *getNodeRefInternal(const char* name, 
                                             const pugi::xml_node& node,
                                             const pugi::xml_node& referringNode,
                                             const pugi::xml_node& ref)
      throw(ParserException);

    static PlexilNodeRef *getLocalNodeRef(const char* name,
                                          const pugi::xml_node& node,
                                          const pugi::xml_node& referringNode,
                                          const pugi::xml_node& ref)
      throw(ParserException);

    static void registerParsers();

    static std::map<std::string, PlexilBodyParser*> *s_bodyParsers;
    static std::map<std::string, PlexilExprParser*> *s_exprParsers;
    static bool s_init;
  };
}

#endif

