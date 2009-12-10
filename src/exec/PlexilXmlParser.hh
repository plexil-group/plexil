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

#ifndef _H_PlexilXmlParser
#define _H_PlexilXmlParser

#include "PlexilPlan.hh"
#include "PlexilResource.hh"
#include "ParserException.hh"
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"
#include <map>

namespace PLEXIL
{

  template<typename Ret>
  class PlexilElementParser
  {
  public:
    virtual Id<Ret> parse(const TiXmlElement* xml)
      throw(ParserException)
      = 0;
    PlexilElementParser()
    {}
    virtual ~PlexilElementParser()
    {}
  };

  typedef PlexilElementParser<PlexilNodeBody> PlexilBodyParser;
  typedef PlexilElementParser<PlexilExpr> PlexilExprParser;

  /**
   * @brief A variation of PlexilExprParser that supports parsing internal variable references.
   */
  class PlexilInternalVarParser : public PlexilExprParser
  {
  public:
    PlexilNodeRefId parseNodeReference(const TiXmlElement* xml);
    PlexilInternalVarParser()
    {}
    virtual ~PlexilInternalVarParser()
    {}
  };



  class PlexilXmlParser : public PlexilParser
  {
  public:
    PlexilXmlParser();
    PlexilXmlParser(const std::string& str, bool isFile)
      throw(ParserException);
    PlexilXmlParser(TiXmlElement* xml);
    ~PlexilXmlParser();

    PlexilNodeId parse(const std::string& str, bool isFile)
      throw(ParserException);
    PlexilNodeId parse(TiXmlElement* xml)
      throw(ParserException);
    PlexilNodeId parse()
      throw(ParserException);

    static PlexilExprId parseExpr(const TiXmlElement* xml)
      throw(ParserException);
    static PlexilNodeId parseNode(const TiXmlElement* node)
      throw(ParserException);
    static PlexilInterfaceId parseDepricatedInterface(const TiXmlElement* intf)
      throw(ParserException);
    static PlexilInterfaceId parseInterface(const TiXmlElement* intf)
       throw(ParserException);
    static void parseInOrInOut(const TiXmlElement* inOrInOut, 
                               PlexilInterfaceId& interface, bool isInOut)
      throw(ParserException);
    static void parseDeclarations(const TiXmlElement* decls, PlexilNodeId& node)
      throw(ParserException);
    static PlexilVar* parseDeclaration(const TiXmlElement* decl)
      throw(ParserException);
    static PlexilVar* parseArrayDeclaration(const TiXmlElement* decls)
      throw(ParserException);
    static PlexilVar* parseAtomicOrStringDeclaration(const TiXmlElement* decls)
      throw(ParserException);
    static PlexilVar* parseDepricatedDeclaration(const TiXmlElement* decls)
      throw(ParserException);
    static PlexilNodeBodyId parseBody(const TiXmlElement* body)
      throw(ParserException);
    static PlexilStateId parseState(const TiXmlElement* xml)
      throw(ParserException);
    static std::vector<PlexilResourceId> parseResource(const TiXmlElement* xml)
      throw(ParserException);

    static PlexilNodeRefId parseNodeRef(const TiXmlElement* xml)
      throw(ParserException);
    static void getNameOrValue(const TiXmlElement* xml, std::string& name, 
			       std::string& value);
    //this is used to get around the old way of handling node references
    static PlexilNodeRefId getNodeRef(const std::string& name, 
				      const TiXmlElement* node)
      throw(ParserException);
    static TiXmlElement* getNodeParent(const TiXmlElement* node);

    static TiXmlElement* toXml(const PlexilNodeId& node)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilInterfaceId& intf)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilVarId& var)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilExprId& expr)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilExpr* expr)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilNodeBodyId& body)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilVarRef* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilOp* op)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilArrayElement* op)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilLookup* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilValue* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilListBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilRequestBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilUpdateBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilAssignmentBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilCommandBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilFunctionCallBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilLibNodeCallBody* ref)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilInternalVar* ref)
      throw(ParserException);
    static void toXml(const PlexilStateId& state, TiXmlElement* parent)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilChangeLookup* lookup)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilFrequencyLookup* lookup)
      throw(ParserException);
    static void toXml(const PlexilUpdateId& update, TiXmlElement* parent)
      throw(ParserException);
    static void toXml(const std::vector<PlexilExpr*>& src, std::vector<TiXmlElement*>& dest)
      throw(ParserException);
    static TiXmlElement* toXml(const PlexilNodeRefId& ref)
      throw(ParserException);

  private:
    static TiXmlElement* element(const std::string& name);
    static TiXmlElement* namedTextElement(const std::string& name, const std::string& value);
    static TiXmlElement* namedNumberElement(const std::string& name, const double value);

    void registerParsers();

    TiXmlElement* m_root;
    bool m_delete;
    static std::map<std::string, PlexilBodyParser*> s_bodyParsers;
    static std::map<std::string, PlexilExprParser*> s_exprParsers;
    static bool s_init;
  };
}

#endif

