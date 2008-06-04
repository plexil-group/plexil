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

#include "PlexilXmlParser.hh"
#include "XMLUtils.hh"
#include "Debug.hh"

#include <sstream>

#define checkTag(t,e) checkParserException(testTag(t, e), "Expected <" << t << "> element, but got <" << e->Value() << "> instead.")
#define checkAttr(t,e) checkParserException(e->Attribute(t) != NULL, "Expected an attribute named '" << t << "' in element <" << e->Value() << ">")
#define checkTagPart(t,e) checkParserException(testTagPart(t, e), "Expected an element containing '" << t << "', but instead got <" << e->Value() << ">")
#define testTagPart(t, e) (std::string(e->Value()).find(t) != std::string::npos)
#define testTag(t, e) (t == e->Value())
#define notEmpty(e) (e->FirstChild() != NULL && e->FirstChild()->Value() != NULL && !std::string(e->FirstChild()->Value()).empty())
#define checkNotEmpty(e) checkParserException(notEmpty(e), "Expected a non-empty text child of <" << e->Value() << ">");
#define hasChildElement(e) (notEmpty(e) && e->FirstChildElement() != NULL)
#define checkHasChildElement(e) checkParserException(hasChildElement(e), "Expected a child element of <" << e->Value() << ">");

using namespace std;

namespace PLEXIL
{
  const std::string NODE_TAG("Node");
  const std::string NODEID_TAG("NodeId");
  const std::string PRIORITY_TAG("Priority");
  const std::string PERMISSIONS_TAG("Permissions");
  const std::string INTERFACE_TAG("Interface");
  const std::string VAR_DECLS_TAG("VariableDeclarations");
  const std::string DECL_VAR_TAG("DeclareVariable");
  const std::string IN_TAG("In");
  const std::string INOUT_TAG("InOut");
  const std::string VAR_TAG("Variable");
  const std::string TYPE_TAG("Type");
  const std::string MAXSIZE_TAG("MaxSize");
  const std::string DECL_TAG("Declare");
  const std::string VAL_TAG("Value");
  const std::string INITIALVAL_TAG("InitialValue");
  const std::string ASSN_TAG("Assignment");
  const std::string BODY_TAG("NodeBody");
  const std::string RHS_TAG("RHS");
  const std::string NODELIST_TAG("NodeList");
  const std::string LIBRARYNODECALL_TAG("LibraryNodeCall");
  const std::string ALIAS_TAG("Alias");
  const std::string NODE_PARAMETER_TAG("NodeParameter");
  const std::string CMD_TAG("Command");
  const std::string CMDNAME_TAG("CommandName");
  const std::string FUNCCALL_TAG("FunctionCall");
  const std::string FUNCCALLNAME_TAG("FunctionName");
  const std::string NAME_TAG("Name");
  const std::string INDEX_TAG("Index");
  const std::string ARGS_TAG("Arguments");
  const std::string LOOKUPNOW_TAG("LookupNow");
  const std::string LOOKUPCHANGE_TAG("LookupOnChange");
  const std::string LOOKUPFREQ_TAG("LookupWithFrequency");
  const std::string FREQ_TAG("Frequency");
  const std::string HIGH_TAG("High");
  const std::string LOW_TAG("Low");
  const std::string TOLERANCE_TAG("Tolerance");
  const std::string NODEREF_TAG("NodeRef");
  const std::string STATEVAL_TAG("NodeStateValue");
  const std::string STATENAME_TAG("StateName");
  const std::string TIMEPOINT_TAG("Timepoint");
  const std::string UPDATE_TAG("Update");
  const std::string REQ_TAG("Request");
  const std::string PAIR_TAG("Pair");
  const std::string COND_TAG("Condition");

  const std::string INT_TAG("Integer");
  const std::string REAL_TAG("Real");
  const std::string BOOL_TAG("Boolean");
  const std::string BLOB_TAG("String");
  const std::string ARRAY_TAG("Array");
  const std::string DECL_ARRAY_TAG("DeclareArray");
  const std::string ARRAYELEMENT_TAG("ArrayElement");
  const std::string STRING_TAG("String");
  const std::string TIME_TAG("Time");

  const std::string NODETYPE_ATTR("NodeType");
  const std::string DIR_ATTR("dir");

  const std::string PARENT_VAL("parent");
  const std::string CHILD_VAL("child");
  const std::string SIBLING_VAL("sibling");
  const std::string SELF_VAL("self");

  bool PlexilXmlParser::s_init = true;
  std::map<std::string, PlexilExprParser*> PlexilXmlParser::s_exprParsers;
  std::map<std::string, PlexilBodyParser*> PlexilXmlParser::s_bodyParsers;

  class PlexilOutcomeVarParser : public PlexilExprParser
  {
  public:
    PlexilOutcomeVarParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilNodeRefId ref;

      //if we have an old-style node reference, we have to do a lot of work!

      if (xml->FirstChildElement(NODEID_TAG) != NULL)
	ref =
	  PlexilXmlParser::getNodeRef(xml->FirstChildElement(NODEID_TAG)->FirstChild()->Value(),
				      PlexilXmlParser::getNodeParent(xml));
      else
	ref = PlexilXmlParser::parseNodeRef(xml->FirstChildElement(NODEREF_TAG));
      PlexilOutcomeVar* retval = new PlexilOutcomeVar();
      retval->setRef(ref);
      return retval->getId();
    }
  };

  class PlexilFailureVarParser : public PlexilExprParser
  {
  public:
    PlexilFailureVarParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilNodeRefId ref;

      //if we have an old-style node reference, we have to do a lot of work!

      if (xml->FirstChildElement(NODEID_TAG) != NULL)
	ref =
	  PlexilXmlParser::getNodeRef(xml->FirstChildElement(NODEID_TAG)->FirstChild()->Value(),
				      PlexilXmlParser::getNodeParent(xml));
      else
	ref = PlexilXmlParser::parseNodeRef(xml->FirstChildElement(NODEREF_TAG));
      PlexilFailureVar* retval = new PlexilFailureVar();
      retval->setRef(ref);
      return retval->getId();
    }
  };

  class PlexilStateVarParser : public PlexilExprParser
  {
  public:
    PlexilStateVarParser(): PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilNodeRefId ref;
      //if we have an old-style node reference, we have to do a lot of work!
      if (xml->FirstChildElement(NODEID_TAG) != NULL)
	ref =
	  PlexilXmlParser::getNodeRef(xml->FirstChildElement(NODEID_TAG)->FirstChild()->Value(),
				      PlexilXmlParser::getNodeParent(xml));
      else
	ref = PlexilXmlParser::parseNodeRef(xml->FirstChildElement(NODEREF_TAG));
      PlexilStateVar* retval = new PlexilStateVar();
      retval->setRef(ref);
      return retval->getId();
    }
  };

  class PlexilCommandHandleVarParser : public PlexilExprParser
  {
  public:
    PlexilCommandHandleVarParser(): PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilNodeRefId ref;
      //if we have an old-style node reference, we have to do a lot of work!
      if (xml->FirstChildElement(NODEID_TAG) != NULL)
	ref =
	  PlexilXmlParser::getNodeRef(xml->FirstChildElement(NODEID_TAG)->FirstChild()->Value(),
				      PlexilXmlParser::getNodeParent(xml));
      else
	ref = PlexilXmlParser::parseNodeRef(xml->FirstChildElement(NODEREF_TAG));
      PlexilCommandHandleVar* retval = new PlexilCommandHandleVar();
      retval->setRef(ref);
      return retval->getId();
    }
  };

  class PlexilTimepointVarParser : public PlexilExprParser
  {
  public:
    PlexilTimepointVarParser(): PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilNodeRefId ref = PlexilNodeRefId::noId();
      //if we have an old-style node reference, we have to do a lot of work!
      if (xml->FirstChildElement(NODEID_TAG) != NULL)
	ref =
	  PlexilXmlParser::getNodeRef(xml->FirstChildElement(NODEID_TAG)->FirstChild()->Value(),
				      PlexilXmlParser::getNodeParent(xml));
      else
	ref = PlexilXmlParser::parseNodeRef(xml->FirstChildElement(NODEREF_TAG));
      PlexilTimepointVar* retval = new PlexilTimepointVar();
      retval->setRef(ref);

      TiXmlElement* state = xml->FirstChildElement(STATEVAL_TAG);
      check_error(state != NULL);
      checkNotEmpty(state);
      retval->setState(state->FirstChild()->Value());

      TiXmlElement* point = xml->FirstChildElement(TIMEPOINT_TAG);
      check_error(point != NULL);
      checkNotEmpty(point);
      retval->setTimepoint(point->FirstChild()->Value());

      return retval->getId();
    }
  };

  class PlexilOpParser : public PlexilExprParser
  {
  public:
    PlexilOpParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilOp* retval = new PlexilOp();
      retval->setOp(xml->Value());
      for (TiXmlElement* child = xml->FirstChildElement(); child != NULL;
	   child = child->NextSiblingElement())
        retval->addSubExpr(PlexilXmlParser::parseExpr(child));
      return retval->getId();
    }
  };

  class PlexilFrequencyLookupParser : public PlexilExprParser
  {
  public:
    PlexilFrequencyLookupParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(LOOKUPFREQ_TAG, xml);
      PlexilFrequencyLookup* retval = new PlexilFrequencyLookup();
      retval->setState(PlexilXmlParser::parseState(xml));
      TiXmlElement* frequencies = xml->FirstChildElement(FREQ_TAG);
      checkParserException(frequencies != NULL, "LookupWithFrequency without a Frequency element!");
      TiXmlElement* freq = frequencies->FirstChildElement(LOW_TAG);
      checkHasChildElement(freq);
      retval->setLowFreq(PlexilXmlParser::parseExpr(freq->FirstChildElement()));
      freq = frequencies->FirstChildElement(HIGH_TAG);
      if (freq != NULL)
	{
	  checkHasChildElement(freq);
	  retval->setHighFreq(PlexilXmlParser::parseExpr(freq->FirstChildElement()));
	}
      return retval->getId();
    }
  };

  class PlexilChangeLookupParser : public PlexilExprParser
  {
  public:
    PlexilChangeLookupParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(LOOKUPCHANGE_TAG, xml);
      PlexilChangeLookup* retval = new PlexilChangeLookup();
      retval->setState(PlexilXmlParser::parseState(xml));
      for (TiXmlElement* tol = xml->FirstChildElement(TOLERANCE_TAG); tol != NULL;
	   tol = tol->NextSiblingElement(TOLERANCE_TAG))
	{
	  checkHasChildElement(tol);
	  retval->addTolerance(PlexilXmlParser::parseExpr(tol->FirstChildElement()));
	}
      return retval->getId();
    }
  };

  class PlexilLookupNowParser : public PlexilExprParser
  {
  public:
    PlexilLookupNowParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(LOOKUPNOW_TAG, xml);
      PlexilLookupNow* retval = new PlexilLookupNow();
      retval->setState(PlexilXmlParser::parseState(xml));
      return retval->getId();
    }
  };

  class PlexilArrayElementParser : public PlexilExprParser
  {
  public:
    PlexilArrayElementParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(ARRAYELEMENT_TAG, xml);
            
      // create an array element
            
      PlexilArrayElement* arrayElement = new PlexilArrayElement();
            
      // extract array name
            
      TiXmlElement* child = xml->FirstChildElement();
      checkTag(NAME_TAG, child);
      std::string name = child->FirstChild()->Value();
      arrayElement->setArrayName(name);

      // extract index

      child = child->NextSiblingElement();
      checkTag(INDEX_TAG, child);
      PlexilExprId indexExpr = 
	PlexilXmlParser::parseExpr(child->FirstChildElement());
      arrayElement->addSubExpr(indexExpr);

      // return new array element

      return arrayElement->getId();
    }
  };

  class PlexilValueParser : public PlexilExprParser
  {
  public:
    PlexilValueParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      // confirm that we have a value

      checkTagPart(VAL_TAG, xml);

      // get value (which could be empty)n

      std::string tag;
      if (xml->Value() != NULL)
	tag = xml->Value();

      // establish value type

      std::string type = tag.substr(0, tag.find(VAL_TAG));

      // establish the value (could be empty)

      std::string value;
      if (xml->FirstChild() != NULL && 
	  xml->FirstChild()->Value() != NULL)
	value = xml->FirstChild()->Value();

      // return new value
      return (new PlexilValue(PlexilXmlParser::toType(type), value))->getId();
    }
  };

  class PlexilVarRefParser : public PlexilExprParser
  {
  public:
    PlexilVarRefParser() : PlexilExprParser()
    {}
    PlexilExprId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTagPart(VAR_TAG, xml);
      checkNotEmpty(xml);
      PlexilVarRef* retval = new PlexilVarRef();
      retval->setName(xml->FirstChild()->Value());
      char* varStart;
      if ((varStart = strstr(xml->Value(), VAR_TAG.c_str())) != xml->Value())
	{
	  std::string type(xml->Value(), varStart - xml->Value());
	  retval->setType(type);
	}
      return retval->getId();
    }
  };

  class PlexilActionParser : public PlexilBodyParser
  {
  public:
    void parseDest(const TiXmlElement* xml, PlexilActionBody* body)
      throw(ParserException)
    {
      for (TiXmlElement* var = xml->FirstChildElement(); var != NULL;
	   var = var->NextSiblingElement())
	{
	  std::string tag(var->Value());
	  if (tag.find(VAR_TAG) != std::string::npos)
            {
              body->addDestVar(PlexilVarRefParser().parse(var));
            }
          else if (tag.find(ARRAYELEMENT_TAG) != std::string::npos)
            {
              body->addDestVar(PlexilArrayElementParser().parse(var));
            }
	    continue;
	}
    }
  };

  class PlexilAssignmentParser : public PlexilActionParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(ASSN_TAG, xml);
      PlexilAssignmentBody* retval = new PlexilAssignmentBody();
      parseDest(xml, retval);
      TiXmlElement* rhs = NULL;
      for (TiXmlElement* child = xml->FirstChildElement(); child != NULL;
	   child = child->NextSiblingElement())
	{
	  std::string tag(child->Value());
	  std::string::size_type pos = tag.find(RHS_TAG);
	  if (pos != std::string::npos)
	    {
	      rhs = child;
	      VarType type = PlexilXmlParser::toType(tag.substr(0, pos));
	      retval->setType(type);
	      break;
	    }
	}
      checkParserException(rhs != NULL, "No RHS for " << *xml);
      checkParserException(rhs->FirstChildElement() != NULL, "Empty RHS for " << *xml);
      retval->setRHS(PlexilXmlParser::parseExpr(rhs->FirstChildElement()));
      return retval->getId();
    }
  };

  class PlexilNodeListParser : public PlexilBodyParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(NODELIST_TAG, xml);
      PlexilListBody* retval = new PlexilListBody();
      for (TiXmlElement* child = xml->FirstChildElement(NODE_TAG); child != NULL;
	   child = child->NextSiblingElement(NODE_TAG))
	retval->addChild(PlexilXmlParser::parseNode(child));
      return retval->getId();
    }
  };
  // parse a library node call

  class PlexilLibraryNodeCallParser : public PlexilBodyParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(LIBRARYNODECALL_TAG, xml);
            
      // get node id

      TiXmlElement* nodeIdXml = xml->FirstChildElement(NODEID_TAG);
      checkParserException(nodeIdXml != NULL, "Missing NodeId element in library call.");
      std::string nodeId(nodeIdXml->FirstChild()->Value());
      checkParserException(!nodeId.empty(), "Empty NodeId element in library call.");
               
      // create lib node call node body

      PlexilLibNodeCallBody* body = new PlexilLibNodeCallBody(nodeId);

      // collect the variable alias information

      for (TiXmlElement* child = xml->FirstChildElement(ALIAS_TAG); child != NULL;
	   child = child->NextSiblingElement(ALIAS_TAG))
	{
	  // get library node parameter 

	  TiXmlElement* libParamXml = child->FirstChildElement(NODE_PARAMETER_TAG);
	  checkParserException(libParamXml != NULL, "Missing NodeParameter element in library call.");
	  std::string libParam(libParamXml->FirstChild()->Value());
	  checkParserException(!libParam.empty(), "Empty NodeParameter element in library call.");
               
	  // get node parameter value

	  PlexilExprId value = PlexilXmlParser::parseExpr(libParamXml->NextSiblingElement());

	  // add alias to body

	  body->addAlias(libParam, value);
	}
      // return lib node call node body

      return body->getId();
    }
  };

  class PlexilCommandParser : public PlexilActionParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(CMD_TAG, xml);
      PlexilCommandBody* retval = new PlexilCommandBody();
      parseDest(xml, retval);
      retval->setState(PlexilXmlParser::parseState(xml));
      retval->setResource(PlexilXmlParser::parseResource(xml));
      return retval->getId();
    }
  };

  class PlexilFunctionCallParser : public PlexilActionParser {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(FUNCCALL_TAG, xml);
      PlexilFunctionCallBody* retval = new PlexilFunctionCallBody();
      parseDest(xml, retval);
      retval->setState(PlexilXmlParser::parseState(xml));
      return retval->getId();
    }
  };

  class PlexilPairsParser : public PlexilBodyParser
  {
  public:
    PlexilUpdateId parsePairs(const TiXmlElement* xml)
      throw(ParserException)
    {
      PlexilUpdateId retval = (new PlexilUpdate())->getId();
      for (TiXmlElement* child = xml->FirstChildElement(PAIR_TAG); child != NULL;
	   child = child->NextSiblingElement(PAIR_TAG))
	{
	  checkNotEmpty(child->FirstChildElement(NAME_TAG));
	  std::string name = child->FirstChildElement(NAME_TAG)->FirstChild()->Value();
	  TiXmlElement* value = child->FirstChildElement();
	  while (value != NULL && value->Value() == NAME_TAG)
	    value = value->NextSiblingElement();
	  checkParserException(value != NULL, "No value in pair at " << *xml);
	  debugMsg("PlexilXml:parsePairs", "Parsed pair {" << name << ", " << *value << "}");
	  retval->addPair(name, PlexilXmlParser::parseExpr(value));
	}
      return retval->getId();
    }
  };

  class PlexilUpdateParser : public PlexilPairsParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(UPDATE_TAG, xml);
      PlexilUpdateBody* retval = new PlexilUpdateBody();
      if (xml->FirstChildElement(PAIR_TAG) != NULL)
	retval->setUpdate(parsePairs(xml));
      return retval->getId();
    }
  };

  class PlexilRequestParser : public PlexilPairsParser
  {
  public:
    PlexilNodeBodyId parse(const TiXmlElement* xml)
      throw(ParserException)
    {
      checkTag(REQ_TAG, xml);
      PlexilRequestBody* retval = new PlexilRequestBody();
      if (xml->FirstChildElement(PAIR_TAG) != NULL)
	retval->setUpdate(parsePairs(xml));
      TiXmlElement* ref = xml->FirstChildElement(NODEREF_TAG);
      if (ref != NULL)
	retval->setParent(PlexilXmlParser::parseNodeRef(ref));
      return retval->getId();
    }
  };

  void PlexilXmlParser::registerParsers()
  {
    s_bodyParsers.insert(std::make_pair(ASSN_TAG, new PlexilAssignmentParser()));
    s_bodyParsers.insert(std::make_pair(NODELIST_TAG, new PlexilNodeListParser()));
    s_bodyParsers.insert(std::make_pair(LIBRARYNODECALL_TAG, new PlexilLibraryNodeCallParser()));
    s_bodyParsers.insert(std::make_pair(CMD_TAG, new PlexilCommandParser()));
    s_bodyParsers.insert(std::make_pair(FUNCCALL_TAG, new PlexilFunctionCallParser()));
    s_bodyParsers.insert(std::make_pair(UPDATE_TAG, new PlexilUpdateParser()));
    s_bodyParsers.insert(std::make_pair(REQ_TAG, new PlexilRequestParser()));

    PlexilExprParser* varRef = new PlexilVarRefParser();
    s_exprParsers.insert(std::make_pair(VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(INT_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(REAL_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(ARRAY_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(STRING_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(BOOL_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(TIME_TAG + VAR_TAG, varRef));
    s_exprParsers.insert(std::make_pair(BLOB_TAG + VAR_TAG, varRef));

    s_exprParsers.insert(std::make_pair("NodeOutcome" + VAR_TAG,
					new PlexilOutcomeVarParser()));
    s_exprParsers.insert(std::make_pair("NodeFailure" + VAR_TAG,
					new PlexilFailureVarParser()));
    s_exprParsers.insert(std::make_pair("NodeState" + VAR_TAG,
					new PlexilStateVarParser()));
    s_exprParsers.insert(std::make_pair("NodeCommandHandle" + VAR_TAG,
					new PlexilCommandHandleVarParser()));
    s_exprParsers.insert(std::make_pair("NodeTimepoint" + VAL_TAG,
					new PlexilTimepointVarParser()));

    PlexilExprParser* val = new PlexilValueParser();
    s_exprParsers.insert(std::make_pair(INT_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(REAL_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(STRING_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(BOOL_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(TIME_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(BLOB_TAG + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair("NodeOutcome" + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair("NodeFailure" + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair("NodeState" + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair("NodeCommandHandle" + VAL_TAG, val));
    s_exprParsers.insert(std::make_pair(LOOKUPNOW_TAG, new PlexilLookupNowParser()));
    s_exprParsers.insert(std::make_pair(LOOKUPCHANGE_TAG, new PlexilChangeLookupParser()));
    s_exprParsers.insert(std::make_pair(LOOKUPFREQ_TAG, new PlexilFrequencyLookupParser()));
    s_exprParsers.insert(std::make_pair(ARRAYELEMENT_TAG, new PlexilArrayElementParser()));

    PlexilExprParser* op = new PlexilOpParser();
    s_exprParsers.insert(std::make_pair("AND", op));
    s_exprParsers.insert(std::make_pair("OR", op));
    s_exprParsers.insert(std::make_pair("XOR", op));
    s_exprParsers.insert(std::make_pair("NOT", op));
    s_exprParsers.insert(std::make_pair("Concat", op));
    s_exprParsers.insert(std::make_pair("IsKnown", op));
    s_exprParsers.insert(std::make_pair("EQ", op));
    s_exprParsers.insert(std::make_pair("EQNumeric", op));
    s_exprParsers.insert(std::make_pair("EQString", op));
    s_exprParsers.insert(std::make_pair("EQBoolean", op));
    s_exprParsers.insert(std::make_pair("EQInternal", op));
    s_exprParsers.insert(std::make_pair("NE", op));
    s_exprParsers.insert(std::make_pair("NENumeric", op));
    s_exprParsers.insert(std::make_pair("NEString", op));
    s_exprParsers.insert(std::make_pair("NEBoolean", op));
    s_exprParsers.insert(std::make_pair("NEInternal", op));
    s_exprParsers.insert(std::make_pair("LT", op));
    s_exprParsers.insert(std::make_pair("LE", op));
    s_exprParsers.insert(std::make_pair("GT", op));
    s_exprParsers.insert(std::make_pair("GE", op));
    s_exprParsers.insert(std::make_pair("ADD", op));
    s_exprParsers.insert(std::make_pair("SUB", op));
    s_exprParsers.insert(std::make_pair("MUL", op));
    s_exprParsers.insert(std::make_pair("DIV", op));
    s_exprParsers.insert(std::make_pair("SQRT", op));
    s_exprParsers.insert(std::make_pair("ABS", op));
  }

  PlexilXmlParser::PlexilXmlParser() : m_root(NULL), m_delete(true)
  {
    if (s_init)
      {
	registerParsers();
	s_init = false;
      }
  }

  PlexilXmlParser::PlexilXmlParser(const std::string& str, bool isFile)
    throw(ParserException)
    : m_delete(true)
  {
    if (isFile)
      {
	TiXmlDocument* doc = new TiXmlDocument(str);
	if (!doc->LoadFile())
	  {
            checkParserException(ALWAYS_FAIL, "Error loading '" << str << "': " << doc->ErrorDesc());
	  }
	m_root = doc->RootElement()->FirstChildElement(NODE_TAG);
      }
    else
      m_root = initXml(str);
    checkParserException(m_root != NULL, "No node root in " << str);
    if (s_init)
      {
	registerParsers();
	s_init = false;
      }
  }

  PlexilXmlParser::PlexilXmlParser(TiXmlElement* xml) : m_root(xml), m_delete(false)
  {
    if (s_init)
      {
	registerParsers();
	s_init = false;
      }
  }

  PlexilNodeId PlexilXmlParser::parse(const std::string& str, bool isFile)
    throw(ParserException)
  {
    if (m_delete && m_root != NULL)
      delete m_root;
    m_delete = true;
    if (isFile)
      {
	TiXmlDocument* doc = new TiXmlDocument(str);
	if (!doc->LoadFile())
	  {
            checkParserException(ALWAYS_FAIL, "Error loading '" << str << "': " << doc->ErrorDesc());
	  }
	m_root = doc->RootElement()->FirstChildElement(NODE_TAG);
      }
    else
      m_root = initXml(str);
    checkParserException(m_root != NULL, "No node root in " << str);
    return parse();
  }

  PlexilNodeId PlexilXmlParser::parse(TiXmlElement* xml)
    throw(ParserException)
  {
    if (m_delete && m_root != NULL)
      delete m_root;
    m_delete = false;
    m_root = xml;
    return parse();
  }
  
  PlexilNodeId PlexilXmlParser::parse()
    throw(ParserException)
  {
    return parseNode(m_root);
  }

  PlexilExprId PlexilXmlParser::parseExpr(const TiXmlElement* xml)
    throw(ParserException)
  {
    std::map<std::string, PlexilExprParser*>::iterator it = 
      s_exprParsers.find(xml->Value());
    checkParserException(it != s_exprParsers.end(),
			 "No parser for expression '" << xml->Value() << "'");
    return it->second->parse(xml);
  }

   PlexilNodeId PlexilXmlParser::parseNode(const TiXmlElement* xml)
      throw(ParserException)
   {
      checkTag(NODE_TAG, xml);
      PlexilNodeId retval = (new PlexilNode())->getId();
      
      // nodeid required
      
      TiXmlElement* nodeIdXml = xml->FirstChildElement(NODEID_TAG);
      checkParserException(nodeIdXml != NULL, "Missing or empty NodeId element.");
      std::string nodeId(nodeIdXml->FirstChild()->Value());
      checkParserException(!nodeId.empty(), "Missing or empty NodeId element.");
      retval->setNodeId(nodeIdXml->FirstChild()->Value());
      
      // node type required
      
      checkAttr(NODETYPE_ATTR, xml);
      retval->setNodeType(xml->Attribute(NODETYPE_ATTR));
      
      // priority optional

      TiXmlElement* priorityXml = xml->FirstChildElement(PRIORITY_TAG);
      if (priorityXml != NULL)
      {
         std::string priority = priorityXml->FirstChild()->Value();
         if (!priority.empty())
         {
            std::stringstream str;
            double value;
            str << priority;
            str >> value;
            retval->setPriority(value);
         }
      }
      
      // permissions optional
      
      TiXmlElement* permissionsXml = xml->FirstChildElement(PERMISSIONS_TAG);
      if (permissionsXml != NULL)
         retval->setPermissions(permissionsXml->FirstChild()->Value());
      
      // interface optional
      
      TiXmlElement* interfaceXml = xml->FirstChildElement(INTERFACE_TAG);
      if (interfaceXml != NULL)
         retval->setInterface(parseInterface(interfaceXml));
      
      // variable declarations optional
      
      TiXmlElement* declarationsXml = xml->FirstChildElement(VAR_DECLS_TAG);
      if (declarationsXml != NULL)
         parseDeclarations(declarationsXml, retval);
      
      // conditions optional
      
      for (TiXmlElement* conditionsXml = xml->FirstChildElement(); 
           conditionsXml != NULL; conditionsXml = conditionsXml->NextSiblingElement())
      {
         std::string tag = conditionsXml->Value();
         if (tag.find(COND_TAG) == std::string::npos)
            continue;
         retval->addCondition(tag, parseExpr(conditionsXml->FirstChildElement()));
      }

      // node body optional
      
      TiXmlElement* bodyXml = xml->FirstChildElement(BODY_TAG);
      if (bodyXml != NULL)
      {
         TiXmlElement* realBodyXml = bodyXml->FirstChildElement();
         if (realBodyXml != NULL)
         {
            retval->setBody(parseBody(realBodyXml));
         }
      }
      
      return retval;
   }

  PlexilInterfaceId PlexilXmlParser::parseDepricatedInterface(
     const TiXmlElement* intf)
     throw(ParserException)
  {
    checkTag(INTERFACE_TAG, intf);
    PlexilInterfaceId retval = (new PlexilInterface())->getId();
    TiXmlElement* in = intf->FirstChildElement(IN_TAG);
    PlexilVarRefParser p;
    if (in != NULL)
      for (TiXmlElement* var = in->FirstChildElement(); var != NULL;
	   var = var->NextSiblingElement())
	retval->addIn(p.parse(var));
    TiXmlElement* inOut = intf->FirstChildElement(INOUT_TAG);
    if (inOut != NULL)
      for (TiXmlElement* var = inOut->FirstChildElement(); var != NULL;
	   var = var->NextSiblingElement())
	retval->addInOut(p.parse(var));
    return retval;
  }
   
   PlexilInterfaceId PlexilXmlParser::parseInterface(const TiXmlElement* intf)
      throw(ParserException)
   {
      PlexilInterfaceId retval = (new PlexilInterface())->getId();
      checkTag(INTERFACE_TAG, intf);
      parseInOrInOut(intf->FirstChildElement(IN_TAG), retval, false);
      parseInOrInOut(intf->FirstChildElement(INOUT_TAG), retval, true);
      return retval;
   }
   
   void PlexilXmlParser::parseInOrInOut(
      TiXmlElement* inOrInOut, PlexilInterfaceId& interface, bool isInOut)
      throw(ParserException)
   {
      // if this is an empty in or inOut section, just return

      if (inOrInOut == NULL)
         return;

      PlexilVarRefParser p; // depricated

      for (TiXmlElement* var = inOrInOut->FirstChildElement(); var != NULL;
           var = var->NextSiblingElement())
      {
         // if this is a declare var or array read those in

         if (testTag(DECL_VAR_TAG, var) || testTag(DECL_ARRAY_TAG, var))
         {
            PlexilVarId variable = parseDeclaration(var)->getId();

            // convert variable to var ref
            
            Id<PlexilVarRef> varRef = (new PlexilVarRef())->getId();
            varRef->setType(variable->type());
            varRef->setName(variable->name());
            varRef->setDefaultValue(variable->value()->getId());

            // add var ref to interface

            if (isInOut)
               interface->addInOut(varRef);
            else
               interface->addIn(varRef);
         }
         
         // else this is the depricated case
         
         else
         {
            warn("DEPRECATED: <" << var->Value() << 
                 "> tag, use <"  << DECL_VAR_TAG <<
                 "> or <"        << DECL_ARRAY_TAG <<
                 "> tag instead.");

            if (isInOut)
               interface->addInOut(p.parse(var));
            else
               interface->addIn(p.parse(var));
         }
      }
   }

   void PlexilXmlParser::parseDeclarations(const TiXmlElement* decls,
                                           PlexilNodeId &node)
      throw(ParserException)
   {
      checkTag(VAR_DECLS_TAG, decls);
      for (TiXmlElement* decl = decls->FirstChildElement(); decl != NULL;
           decl = decl->NextSiblingElement())
         node->addVariable(parseDeclaration(decl)->getId());
   }

   PlexilVar* PlexilXmlParser::parseDeclaration(const TiXmlElement* decl)
      throw(ParserException)
   {
      // if array declaration
      
      if (testTag(DECL_ARRAY_TAG, decl))
         return parseArrayDeclaration(decl);
      
      // if new vairaible declaration xml syntax
      
      if (testTag(DECL_VAR_TAG, decl))
         return parseAtomicOrStringDeclaration(decl);
      
      // else its a deprecated varible declairation xml syntax
      
      return parseDepricatedDeclaration(decl);
   }

  // parse an array declaration

   PlexilVar* PlexilXmlParser::parseArrayDeclaration(const TiXmlElement* decl)
    throw(ParserException)
  {
    checkTag(DECL_ARRAY_TAG, decl);

    // extract array name
            
    TiXmlElement* child = decl->FirstChildElement();
    checkTag(NAME_TAG, child);
    std::string name = child->FirstChild()->Value();

    // extract array type

    child = child->NextSiblingElement();
    checkTag(TYPE_TAG, child);
    std::string type = child->FirstChild()->Value();

    // extract array max size

    child = child->NextSiblingElement();
    checkTag(MAXSIZE_TAG, child);
    std::stringstream maxSizeStr;
    maxSizeStr << child->FirstChild()->Value();
    unsigned int maxSize;
    maxSizeStr >> maxSize;

    // if present, extract initial values

    std::vector<std::string> initVals;
    if ((child = child->NextSiblingElement()) != NULL)
      {
	checkTag(INITIALVAL_TAG, child);
	child = child->FirstChildElement();
	do
	  {
            checkTagPart(VAL_TAG, child);
            std::string initValTag = std::string(child->Value());
            std::string initValType = initValTag
	      .substr(0, initValTag.size() - VAL_TAG.size());
            checkParserException(type == initValType,
				 "Initial value of " << type << " array variable \'" <<
				 name << "\' of incorrect type \'" << initValType << "\'");
            std::string initVal = child->FirstChild()->Value();
            initVals.push_back(initVal);
            checkParserException(initVals.size() <= maxSize,
				 "Number of initial values of " << type << 
				 " array variable \'" << name << 
				 "\' exceeds maximun of " << maxSize);
	  }
	while ((child = child->NextSiblingElement()) != NULL);
      }

    // add varaible to retruned variable set
      
    return new PlexilArrayVar(name, type, maxSize, initVals);
  }

   // parse an atomic or string declaration
   
   PlexilVar* PlexilXmlParser::parseAtomicOrStringDeclaration(
      const TiXmlElement* decl)
      throw(ParserException)
   {
      checkTag(DECL_VAR_TAG, decl);

      // extract name
      
      TiXmlElement* child = decl->FirstChildElement();
      checkTag(NAME_TAG, child);
      std::string name = child->FirstChild()->Value();
      
      // extract type
      
      child = child->NextSiblingElement();
      checkTag(TYPE_TAG, child);
      std::string type = child->FirstChild()->Value();
      
      // if present, create variable with initial value
      
      if ((child = child->NextSiblingElement()) != NULL)
      {
         checkTag(INITIALVAL_TAG, child);
         child = child->FirstChildElement();
         checkTagPart(VAL_TAG, child);
         std::string initValTag = std::string(child->Value());
         std::string initValType = initValTag
            .substr(0, initValTag.size() - VAL_TAG.size());
         checkParserException(type == initValType,
                              "Initial value of " << type << " variable \'" <<
                              name << "\' of incorrect type \'" << 
                              initValType << "\'");
         return new PlexilVar(name, type, child->FirstChild()->Value());
      }
      
      // otherwise create varible with the value unknown
      
      return new PlexilVar(name, type);
   }
   
  // parse a depricated declaration

   PlexilVar* PlexilXmlParser::parseDepricatedDeclaration(
      const TiXmlElement* decl)
    throw(ParserException)
  {
    checkTagPart(DECL_TAG, decl);
    std::string tag(decl->Value());
    VarType type = toType(tag.substr(DECL_TAG.size()));
    std::string name;
    std::string value;
            
    TiXmlElement* child = decl->FirstChildElement();
    getNameOrValue(child, name, value);
    child = child->NextSiblingElement();
    getNameOrValue(child, name, value);
    
    warn("DEPRECATED: <" << decl->Value() << 
	 "> tag, use <DeclareVariable> tag instead.");

    checkParserException(!name.empty(),
			 "Must have a tag ending in '" << 
			 VAR_TAG << "' as a child of a <" <<
			 VAR_DECLS_TAG << "> element.");
    if (value.empty())
       return new PlexilVar(name, type);
    else
       return new PlexilVar(name, type, value);
  }

  PlexilNodeBodyId PlexilXmlParser::parseBody(const TiXmlElement* body)
    throw(ParserException)
  {
    std::string name(body->Value());
    std::map<std::string, PlexilBodyParser*>::iterator it = s_bodyParsers.find(name);
    checkParserException(it != s_bodyParsers.end(), "No parser for body type " << name);
    return it->second->parse(body);
  }

  PlexilStateId PlexilXmlParser::parseState(const TiXmlElement* xml)
    throw(ParserException)
  {
    PlexilStateId retval = (new PlexilState())->getId();
    TiXmlElement* arguments = NULL;
    for (TiXmlElement* child = xml->FirstChildElement(); child != NULL;
	 child = child->NextSiblingElement())
      {
	// create tag string

	std::string tag(child->Value());

	// if state or command name, use contained value

	if (tag == STATENAME_TAG || tag == CMDNAME_TAG || tag == FUNCCALLNAME_TAG)
	  {
            checkNotEmpty(child);
            retval->setName(child->FirstChild()->Value());
	  }
	// if name, it should contain a string variable or value expression
         
	else if (tag == NAME_TAG)
	  {
            checkHasChildElement(child);
            retval->setNameExpr(parseExpr(child->FirstChildElement()));
	  }
	// if it's an argument use that

	else if (tag == ARGS_TAG)
	  arguments = child;
      }

    if (arguments != NULL)
      {
	for (TiXmlElement* child = arguments->FirstChildElement(); child != NULL;
	     child = child->NextSiblingElement())
          {
            retval->addArg(PlexilXmlParser::parseExpr(child));
          }
      }

    return retval;
  }

  std::vector<PlexilResourceId> PlexilXmlParser::parseResource(const TiXmlElement* xml)
    throw(ParserException)
  {
    checkTag(CMD_TAG, xml);

    // Create a vector of PlexilResourceId
    std::vector<PlexilResourceId> rId_vec;
    
    for (TiXmlElement* child = xml->FirstChildElement(); child != NULL;
	 child = child->NextSiblingElement())
      {
	std::string tag(child->Value());
        // Parse only resource list tag
        if (tag == RESOURCELIST_TAG)
          {
            // Loop through each resource in the list
            for (TiXmlElement* child2 = child->FirstChildElement(); child2 != NULL;
                 child2 = child2->NextSiblingElement())
              {
                // Create a new PlexilResourceId.
                PlexilResourceId prId = (new PlexilResource())->getId();
                // loop through each resource element
                for (TiXmlElement* child3 = child2->FirstChildElement(); child3 != NULL;
                     child3 = child3->NextSiblingElement())
                  {
                    // add each resource element just like addArg to PLexilResourceId. Use
                    // tag3 and expresssion the in <name, expr> pair
                    prId->addResource(child3->Value(), PlexilXmlParser::parseExpr(child3->FirstChildElement()));
                  }

                // push the PlexilResourceId into a vector to be returned and 
                // used in the PlexilCommandBody.
                rId_vec.push_back(prId);
              }
          }
      }
    
    return rId_vec;
    // return the vector of PlexilResourceId
  }

  PlexilNodeRefId PlexilXmlParser::parseNodeRef(const TiXmlElement* ref)
    throw(ParserException)
  {
    checkTag(NODEREF_TAG, ref);
    checkAttr(DIR_ATTR, ref);
    PlexilNodeRefId retval = (new PlexilNodeRef())->getId();
    if (ref->Attribute(DIR_ATTR) == CHILD_VAL)
      retval->setDir(PlexilNodeRef::CHILD);
    else if (ref->Attribute(DIR_ATTR) == PARENT_VAL)
      retval->setDir(PlexilNodeRef::PARENT);
    else if (ref->Attribute(DIR_ATTR) == SIBLING_VAL)
      retval->setDir(PlexilNodeRef::SIBLING);
    else if (ref->Attribute(DIR_ATTR) == SELF_VAL)
      retval->setDir(PlexilNodeRef::SELF);
    else
      {
	checkParserException(ALWAYS_FAIL,
			     "Invalid value for 'dir' attibute: " << ref->Attribute(DIR_ATTR));
      }
    if (retval->dir() != PlexilNodeRef::PARENT && retval->dir() != PlexilNodeRef::SELF)
      {
	checkNotEmpty(ref);
	retval->setName(ref->FirstChild()->Value());
      }
    return retval;
  }

  VarType PlexilXmlParser::toType(const std::string& typeStr)
  {
    return typeStr;
  }

  void PlexilXmlParser::getNameOrValue(const TiXmlElement* xml, 
				       std::string& name,
				       std::string& value)
  {
    if (xml == NULL)
      return;
    std::string tag(xml->Value());
    if (tag.find(VAR_TAG) != std::string::npos)
      name = xml->FirstChild()->Value();
    else if (tag.find(VAL_TAG) != std::string::npos)
      {
	if (!xml->NoChildren())
	  value = xml->FirstChild()->Value();
      }
  }

  TiXmlElement* PlexilXmlParser::getNodeParent(const TiXmlElement* node)
  {
    if (node->Parent() == NULL)
      return NULL;
    if (node->Parent()->Value() == NODE_TAG)
      return (TiXmlElement*) node->Parent();
    return getNodeParent((TiXmlElement*) node->Parent());
  }

  PlexilNodeRefId PlexilXmlParser::getNodeRef(const std::string& name,
					      const TiXmlElement* node)
    throw(ParserException)
  {
    TiXmlElement* checkSelf = NULL, * checkParent = NULL, * checkSibling = NULL,
      * checkChild = NULL;
    checkTag(NODE_TAG, node);

    TiXmlElement* selfId = node->FirstChildElement(NODEID_TAG);
    if (selfId != NULL && selfId->FirstChild() != NULL &&
	name == selfId->FirstChild()->Value()) {
      debugMsg("PlexilXmlParser:getNodeRef",
	       "Found self with name " << name);
      checkSelf = const_cast<TiXmlElement*>(node);
    }

    TiXmlElement* parent = getNodeParent(node);
    if (parent != NULL)
      {

	//find parent with name
	TiXmlElement* parentId = parent->FirstChildElement(NODEID_TAG);
	if (parentId != NULL && parentId->FirstChild() != NULL &&
	    parentId->FirstChild()->Value() == name) {
	  checkParent = (TiXmlElement*) node->Parent();
	  debugMsg("PlexilXmlParser:getNodeRef",
		   "Found parent with name " << name);
	}

	//get siblings with name
	for (checkSibling = parent->FirstChildElement(BODY_TAG)
	       ->FirstChildElement(NODELIST_TAG)
	       ->FirstChildElement(NODE_TAG); checkSibling != NULL;
	     checkSibling = checkSibling->NextSiblingElement(NODE_TAG))
	  {
            TiXmlElement* siblingId = checkSibling->FirstChildElement(NODEID_TAG);
            if (checkSibling != checkSelf && siblingId != NULL &&
		siblingId->FirstChild() != NULL && siblingId->FirstChild()->Value() == name) {
	      debugMsg("PlexilXmlParser:getNodeRef",
		       "Found sibling with name " << name);
	      break;
	    }
	  }
      }

    //get children with name
    if (node->FirstChildElement(BODY_TAG) != NULL &&
	node->FirstChildElement(BODY_TAG)->FirstChildElement(NODELIST_TAG) != NULL)
      {
	for (checkChild = node->FirstChildElement(BODY_TAG)
	       ->FirstChildElement(NODELIST_TAG)
	       ->FirstChildElement(NODE_TAG); checkChild != NULL;
	     checkChild = checkChild->NextSiblingElement(NODE_TAG))
	  {
            TiXmlElement* childId = checkChild->FirstChildElement(NODEID_TAG);
            if (childId != NULL && childId->FirstChild() != NULL &&
                childId->FirstChild()->Value() == name) {
	      debugMsg("PlexilXmlParser:getNodeRef",
		       "Found child with name " << name);
	      break;
	    }
	  }
      }

    PlexilNodeRefId retval;
    if (checkSelf != NULL)
      {
	retval = (new PlexilNodeRef())->getId();
	retval->setDir(PlexilNodeRef::SELF);
      }
    if (checkParent != NULL)
      {
	checkParserException(!retval.isValid(),
			     "Ambiguous old-style node reference.  Node " << (*node) <<
			     " and its parent are both named '" << name << "'");
	retval = (new PlexilNodeRef())->getId();
	retval->setDir(PlexilNodeRef::PARENT);
      }
    if (checkSibling != NULL)
      {
	checkParserException(!retval.isValid(),
			     "Ambiguous old-style node reference.  Node " << (*node) <<
			     " has a sibling and either a parent or itself named '" << name << "'");
	retval = (new PlexilNodeRef())->getId();
	retval->setDir(PlexilNodeRef::SIBLING);

      }
    if (checkChild != NULL)
      {
	checkParserException(!retval.isValid(),
			     "Ambiguous old-style node reference.  Node " << (*node) <<
			     " has a sibling, parent, or itself and a child named '" << name << "'");
	retval = (new PlexilNodeRef())->getId();
	retval->setDir(PlexilNodeRef::CHILD);
      }
    checkParserException(retval.isValid(),
			 "No node named '" << name << "' in vicinity of " << (*node));
    retval->setName(name);
    return retval;
  }

  PlexilXmlParser::~PlexilXmlParser()
  {
    if (m_delete && m_root != NULL)
      delete m_root;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeId& node)
    throw(ParserException)
  {
    TiXmlElement* retval = new TiXmlElement(NODE_TAG);
    retval->SetAttribute(NODETYPE_ATTR, node->nodeType());

    retval->InsertEndChild(namedTextElement(NODEID_TAG, node->nodeId()));
    retval->InsertEndChild(namedTextElement(PERMISSIONS_TAG, node->permissions()));
    retval->InsertEndChild(namedNumberElement(PRIORITY_TAG, node->priority()));

    if (node->interface().isValid())
      retval->InsertEndChild(toXml(node->interface()));

    if (!node->declarations().empty())
      {
	TiXmlElement* declarations = element(VAR_DECLS_TAG);
	for (std::vector<PlexilVarId>::const_iterator it = node->declarations().begin();
	     it != node->declarations().end(); ++it)
	  declarations->InsertEndChild(toXml(*it));
	retval->InsertEndChild(declarations);
      }

    for (std::map<std::string, PlexilExprId>::const_iterator it = node->conditions().begin();
	 it != node->conditions().end(); ++it)
      {
	TiXmlElement* cond = element(it->first);
	cond->InsertEndChild(toXml(it->second));
	retval->InsertEndChild(cond);
      }

    if (node->body().isValid())
      retval->InsertEndChild(toXml(node->body()));

    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilInterfaceId& intf)
    throw(ParserException)
  {
    TiXmlElement* retval = element(INTERFACE_TAG);
    if (!intf->in().empty())
      {
	TiXmlElement* in = element(IN_TAG);
	for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
	     it != intf->in().end(); ++it)
        {
           in->InsertEndChild(toXml(*it));
        }
	retval->InsertEndChild(in);
      }
    if (!intf->inOut().empty())
      {
	TiXmlElement* inOut = element(INOUT_TAG);
	for (std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
	     it != intf->inOut().end(); ++it)
        {
           inOut->InsertEndChild(toXml(*it));
        }
	retval->InsertEndChild(inOut);
      }
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilVarId& var)
    throw(ParserException)
  {
    TiXmlElement* retval = element(std::string(DECL_TAG) + var->type());
    TiXmlElement* varName = namedTextElement(var->type() + VAR_TAG, var->name());
    retval->InsertEndChild(varName);
    retval->InsertEndChild(toXml(var->value()->getId()));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilExprId& expr)
    throw(ParserException)
  {
    return toXml(expr.operator->());
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilExpr* expr)
    throw(ParserException)
  {
    if (0 != dynamic_cast<const PlexilVarRef*>(expr))
      return toXml((const PlexilVarRef*) expr);
    else if (0 != dynamic_cast<const PlexilOp*>(expr))
      return toXml((const PlexilOp*) expr);
    else if (0 != dynamic_cast<const PlexilArrayElement*>(expr))
      return toXml((const PlexilArrayElement*) expr);
    else if (0 != dynamic_cast<const PlexilLookup*>(expr))
      return toXml((const PlexilLookup*) expr);
    else if (0 != dynamic_cast<const PlexilValue*>(expr))
      return toXml((const PlexilValue*) expr);
    checkParserException(ALWAYS_FAIL, "Should never get here.");
    return NULL;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeBodyId& body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(BODY_TAG);
    TiXmlElement* realBody;
    if (Id<PlexilListBody>::convertable(body))
      realBody = toXml((PlexilListBody*) body);
    else if (Id<PlexilRequestBody>::convertable(body))
      realBody = toXml((PlexilRequestBody*) body);
    else if (Id<PlexilUpdateBody>::convertable(body))
      realBody = toXml((PlexilUpdateBody*) body);
    else if (Id<PlexilAssignmentBody>::convertable(body))
      realBody = toXml((PlexilAssignmentBody*) body);
    else if (Id<PlexilCommandBody>::convertable(body))
      realBody = toXml((PlexilCommandBody*)body);
    else if (Id<PlexilFunctionCallBody>::convertable(body))
      realBody = toXml((PlexilFunctionCallBody*)body);
    else if (Id<PlexilLibNodeCallBody>::convertable(body))
      realBody = toXml((PlexilLibNodeCallBody*)body);
    checkParserException(realBody != NULL, "Unknown body type.");
    retval->InsertEndChild(realBody);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilVarRef* ref)
    throw(ParserException)
  {
    if (Id<PlexilInternalVar>::convertable(ref->getId()))
      return PlexilXmlParser::toXml((PlexilInternalVar*) ref);
    return namedTextElement((ref->typed() ? ref->type() + VAR_TAG : VAR_TAG),
			    ref->name());
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilOp* op)
    throw(ParserException)
  {
    TiXmlElement* retval = element(op->name());
    for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin();
	 it != op->subExprs().end(); ++it)
      retval->InsertEndChild(toXml(*it));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilArrayElement* op)
    throw(ParserException)
  {
    TiXmlElement* retval = element(ARRAYELEMENT_TAG);
    retval->InsertEndChild(namedTextElement(NAME_TAG, op->getArrayName()));
    for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin();
	 it != op->subExprs().end(); ++it)
      retval->InsertEndChild(toXml(*it));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilLookup* lookup)
    throw(ParserException)
  {
    TiXmlElement* retval;
    if (Id<PlexilLookupNow>::convertable(lookup->getId()))
      retval = element(LOOKUPNOW_TAG);
    else if (Id<PlexilChangeLookup>::convertable(lookup->getId()))
      retval = toXml((PlexilChangeLookup*)lookup);
    else if (Id<PlexilFrequencyLookup>::convertable(lookup->getId()))
      retval = toXml((PlexilFrequencyLookup*)lookup);
    check_error(retval != NULL, "Unknown lookup type.");
    toXml(lookup->state(), retval);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilValue* val)
    throw(ParserException)
  {
    return namedTextElement(val->type() + VAL_TAG, val->value());
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilListBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(NODELIST_TAG);
    for (std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
	 it != body->children().end(); ++it)
      retval->InsertEndChild(toXml(*it));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilRequestBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(REQ_TAG);
    if (body->parent().isValid())
      retval->InsertEndChild(toXml(body->parent()));
    toXml(body->update(), retval);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilUpdateBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(UPDATE_TAG);
    toXml(body->update(), retval);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilAssignmentBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(ASSN_TAG);
    std::vector<TiXmlElement*> dest;
    toXml(body->dest(), dest);
    for (std::vector<TiXmlElement*>::const_iterator it = dest.begin(); it != dest.end(); ++it)
      retval->InsertEndChild(*it);
    TiXmlElement* rhs = element(RHS_TAG);
    rhs->InsertEndChild(toXml(body->RHS()));
    retval->InsertEndChild(rhs);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilCommandBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(CMD_TAG);
    std::vector<TiXmlElement*> dest;
    toXml(body->dest(), dest);
    for (std::vector<TiXmlElement*>::const_iterator it = dest.begin(); it != dest.end(); ++it)
      retval->InsertEndChild(*it);
    toXml(body->state(), retval);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilFunctionCallBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(FUNCCALL_TAG);
    std::vector<TiXmlElement*> dest;
    toXml(body->dest(), dest);
    for(std::vector<TiXmlElement*>::const_iterator it = dest.begin(); it != dest.end(); ++it)
      retval->InsertEndChild(*it);

    toXml(body->state(), retval);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilLibNodeCallBody* body)
    throw(ParserException)
  {
    TiXmlElement* retval = element(LIBRARYNODECALL_TAG);
    retval->InsertEndChild(namedTextElement(NODEID_TAG, body->libNodeName()));

    // format variable aliases
    for (PlexilAliasMap::const_iterator it = body->aliases().begin();
	 it != body->aliases().end();
	 it++)
      {
	// double is key to LabelStr of formal param name
	// expr is actual param
	const std::pair<double, PlexilExprId>& entry = *it;
	TiXmlElement* aliasXml = element(ALIAS_TAG);
	aliasXml->InsertEndChild(namedTextElement(NODE_PARAMETER_TAG, LabelStr(entry.first).toString()));
	aliasXml->InsertEndChild(toXml(entry.second));
	retval->InsertEndChild(aliasXml);
      }

    // linked library node currently ignored
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilInternalVar* var)
    throw(ParserException)
  {
    std::string name(NODE_TAG);
    if (Id<PlexilOutcomeVar>::convertable(var->getId()))
      name.append("Outcome");
    else if (Id<PlexilFailureVar>::convertable(var->getId()))
      name.append("Failure");
    else if (Id<PlexilStateVar>::convertable(var->getId()))
      name.append("State");
    else if (Id<PlexilCommandHandleVar>::convertable(var->getId()))
      name.append("CommandHandle");
    else if (Id<PlexilTimepointVar>::convertable(var->getId()))
      name.append("Timepoint");
    name.append(VAR_TAG);
    TiXmlElement* retval = element(name);
    retval->InsertEndChild(toXml(var->ref()));

    if (Id<PlexilTimepointVar>::convertable(var->getId()))
      {
	PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
	retval->InsertEndChild(namedTextElement(STATEVAL_TAG, tp->state()));
	retval->InsertEndChild(namedTextElement(TIMEPOINT_TAG, tp->timepoint()));
      }
    return retval;
  }

  void PlexilXmlParser::toXml(const PlexilStateId& state, TiXmlElement* parent)
    throw(ParserException)
  {
    parent->InsertEndChild(namedTextElement(NAME_TAG, state->name()));
    TiXmlElement* args = element(ARGS_TAG);
    for (std::vector<PlexilExprId>::const_iterator it = state->args().begin();
	 it != state->args().end(); ++it)
      args->InsertEndChild(toXml(*it));
    parent->InsertEndChild(args);
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilChangeLookup* lookup)
    throw(ParserException)
  {
    TiXmlElement* retval = element(LOOKUPCHANGE_TAG);
    for (std::vector<PlexilExprId>::const_iterator it = lookup->tolerances().begin();
	 it != lookup->tolerances().end(); ++it)
      retval->InsertEndChild(toXml(*it));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilFrequencyLookup* lookup)
    throw(ParserException)
  {
    TiXmlElement* retval = element(LOOKUPFREQ_TAG);
    TiXmlElement* freq = element(FREQ_TAG);
    TiXmlElement* low = element(LOW_TAG);
    low->InsertEndChild(toXml(lookup->lowFreq()));
    freq->InsertEndChild(low);

    if (lookup->highFreq().isValid())
      {
	TiXmlElement* high = element(HIGH_TAG);
	high->InsertEndChild(toXml(lookup->highFreq()));
	freq->InsertEndChild(high);
      }
    retval->InsertEndChild(freq);
    return retval;
  }

  void PlexilXmlParser::toXml(const PlexilUpdateId& update, TiXmlElement* parent)
    throw(ParserException)
  {
    for (std::vector<std::pair<std::string, PlexilExprId> >::const_iterator it =
	   update->pairs().begin(); it != update->pairs().end(); ++it)
      {
	TiXmlElement* pair = element(PAIR_TAG);
	pair->InsertEndChild(namedTextElement(NAME_TAG, it->first));
	pair->InsertEndChild(toXml(it->second));
	parent->InsertEndChild(pair);
      }
  }

  void PlexilXmlParser::toXml(const std::vector<PlexilExpr*>& src,
			      std::vector<TiXmlElement*>& dest)
    throw(ParserException)
  {
    for (std::vector<PlexilExpr*>::const_iterator it = src.begin(); it != src.end(); ++it)
      dest.push_back(toXml(*it));
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeRefId& ref)
    throw(ParserException)
  {
    std::string dir("");
    switch (ref->dir())
      {
      case PlexilNodeRef::SELF:
	dir="self";
	break;
      case PlexilNodeRef::PARENT:
	dir = "parent";
	break;
      case PlexilNodeRef::CHILD:
	dir = "child";
	break;
      case PlexilNodeRef::SIBLING:
	dir = "sibling";
	break;
      default:
	checkParserException(ALWAYS_FAIL, "Unknown direction " << ref->dir());
	break;
      }
    TiXmlElement* retval;
    if (ref->dir() == PlexilNodeRef::CHILD || ref->dir() == PlexilNodeRef::SIBLING)
      retval = namedTextElement(NODEREF_TAG, ref->name());
    else
      retval = element(NODEREF_TAG);
    retval->SetAttribute(DIR_ATTR, dir);
    return retval;
  }

  TiXmlElement* PlexilXmlParser::element(const std::string& name)
  {
    return new TiXmlElement(name);
  }

  TiXmlElement* PlexilXmlParser::namedTextElement(const std::string& name,
						  const std::string& value)
  {
    TiXmlElement* retval = element(name);
    retval->InsertEndChild(new TiXmlText(value));
    return retval;
  }

  TiXmlElement* PlexilXmlParser::namedNumberElement(const std::string& name,
						    const double value)
  {
    std::stringstream str;
    str << value;
    return namedTextElement(name, str.str());
  }
}
