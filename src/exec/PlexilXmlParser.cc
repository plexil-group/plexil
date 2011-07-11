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

#include <set>
#include <sstream>

using namespace std;

namespace PLEXIL 
{
  const std::string PLEXIL_PLAN_TAG("PlexilPlan");
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
  const std::string ARRAY_VAL_TAG("ArrayValue");
  const std::string INITIALVAL_TAG("InitialValue");
  const std::string ASSN_TAG("Assignment");
  const std::string BODY_TAG("NodeBody");
  const std::string RHS_TAG("RHS");
  const std::string NODELIST_TAG("NodeList");
  const std::string LIBRARYNODECALL_TAG("LibraryNodeCall");
  const std::string ALIAS_TAG("Alias");
  const std::string NODE_PARAMETER_TAG("NodeParameter");
  const std::string CMD_TAG("Command");
  const std::string NAME_TAG("Name");
  const std::string INDEX_TAG("Index");
  const std::string ARGS_TAG("Arguments");
  const std::string LOOKUPNOW_TAG("LookupNow");
  const std::string LOOKUPCHANGE_TAG("LookupOnChange");
  const std::string TOLERANCE_TAG("Tolerance");
  const std::string NODEREF_TAG("NodeRef");
  const std::string STATEVAL_TAG("NodeStateValue");
  const std::string TIMEPOINT_TAG("Timepoint");
  const std::string UPDATE_TAG("Update");
  const std::string REQ_TAG("Request");
  const std::string PAIR_TAG("Pair");
  const std::string COND_TAG("Condition");

  const std::string INT_TAG("Integer");
  const std::string REAL_TAG("Real");
  const std::string BOOL_TAG("Boolean");
  const std::string BLOB_TAG("BLOB");
  const std::string STRING_TAG("String");
  const std::string TIME_TAG("Time");
  const std::string ARRAY_TAG("Array");

  const std::string NODE_OUTCOME_TAG("NodeOutcome");
  const std::string NODE_FAILURE_TAG("NodeFailure");
  const std::string NODE_STATE_TAG("NodeState");
  const std::string NODE_COMMAND_HANDLE_TAG("NodeCommandHandle");

  const std::string DECL_ARRAY_TAG("DeclareArray");
  const std::string ARRAYELEMENT_TAG("ArrayElement");

  const std::string NODETYPE_ATTR("NodeType");
  const std::string DIR_ATTR("dir");

  const std::string FILENAME_ATTR("FileName");
  const std::string LINENO_ATTR("LineNo");
  const std::string COLNO_ATTR("ColNo");

  const std::string PARENT_VAL("parent");
  const std::string CHILD_VAL("child");
  const std::string SIBLING_VAL("sibling");
  const std::string SELF_VAL("self");

  //
  // Internal error checking/reporting utilities
  //

  inline bool testTagPart(const string& t, const TiXmlNode* e) {
	return std::string(e->Value()).find(t) != std::string::npos;
  }

  inline bool testTag(const string& t, const TiXmlNode* e) {
	return t == e->Value();
  }

  inline bool notEmpty(const TiXmlNode* e) {
	return e->FirstChild() != NULL && e->FirstChild()->Value() != NULL
	  && !std::string(e->FirstChild()->Value()).empty();
  }

  inline bool hasChildElement(const TiXmlNode* e) {
	return notEmpty(e) && e->FirstChildElement() != NULL;
  }

  void checkTag(const string& t, const TiXmlNode* e) {
	checkParserExceptionWithLocation(testTag(t, e),
									 e,
									 "XML parsing error: Expected <" << t << "> element, but got <" << e->Value() << "> instead.");
  }

  void checkAttr(const string& t, const TiXmlElement* e) {
	checkParserExceptionWithLocation(e->Attribute(t) != NULL,
									 e,
									 "XML parsing error: Expected an attribute named '" << t << "' in element <" << e->Value() << ">");
  }

  void checkTagPart(const string& t, const TiXmlNode* e) {
	checkParserExceptionWithLocation(testTagPart(t, e),
									 e,
									 "XML parsing error: Expected an element containing '" << t << "', but instead got <" << e->Value() << ">");
  }

  void checkNotEmpty(const TiXmlNode* e) {
	checkParserExceptionWithLocation(notEmpty(e),
									 e,
									 "XML parsing error: Expected a non-empty text child of <" << e->Value() << ">");
  }

  void checkHasChildElement(const TiXmlNode* e) {
	checkParserExceptionWithLocation(hasChildElement(e),
									 e,
									 "XML parsing error: Expected a child element of <" << e->Value() << ">");
  }

  //
  // Implementation of parser class methods
  //

  bool PlexilXmlParser::s_init = true;
  std::map<std::string, PlexilExprParser*> *PlexilXmlParser::s_exprParsers = NULL;
  std::map<std::string, PlexilBodyParser*> *PlexilXmlParser::s_bodyParsers = NULL;

  /**
   * @brief A variation of PlexilExprParser that supports parsing internal variable references.
   */
  class PlexilInternalVarParser : public PlexilExprParser
  {
  public:
    PlexilInternalVarParser() : PlexilExprParser() {};
    virtual ~PlexilInternalVarParser() {}

	static PlexilNodeRefId parseNodeReference(const TiXmlElement* xml) 
	{
	  const TiXmlElement* child = xml->FirstChildElement(NODEID_TAG);
	  //if we have an old-style node reference, we have to do a lot of work!
	  if (child != NULL)
		return PlexilXmlParser::getNodeRef(child,
										   PlexilXmlParser::getNodeParent(xml));
	  else if ((child = xml->FirstChildElement(NODEREF_TAG)) != NULL)
		return PlexilXmlParser::parseNodeRef(child);
	  else {
		checkParserExceptionWithLocation(ALWAYS_FAIL,
										 xml,
										 "XML parsing error: Internal variable reference lacks "
										 << NODEID_TAG << " or "
										 << NODEREF_TAG << " tag");

		return PlexilNodeRefId::noId();
	  }
	}
  };



  class PlexilOutcomeVarParser: public PlexilInternalVarParser {
  public:
	PlexilOutcomeVarParser() : PlexilInternalVarParser() {}
	~PlexilOutcomeVarParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilOutcomeVar* retval = new PlexilOutcomeVar();
	  retval->setRef(parseNodeReference(xml));
	  return retval->getId();
	}
  };

  class PlexilFailureVarParser: public PlexilInternalVarParser {
  public:
	PlexilFailureVarParser() : PlexilInternalVarParser() {}
	~PlexilFailureVarParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilFailureVar* retval = new PlexilFailureVar();
	  retval->setRef(parseNodeReference(xml));
	  return retval->getId();
	}
  };

  class PlexilStateVarParser: public PlexilInternalVarParser {
  public:
	PlexilStateVarParser() : PlexilInternalVarParser() {}
	~PlexilStateVarParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilStateVar* retval = new PlexilStateVar();
	  retval->setRef(parseNodeReference(xml));
	  return retval->getId();
	}
  };

  class PlexilCommandHandleVarParser: public PlexilInternalVarParser {
  public:
	PlexilCommandHandleVarParser() : PlexilInternalVarParser() {}
	~PlexilCommandHandleVarParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilCommandHandleVar* retval = new PlexilCommandHandleVar();
	  retval->setRef(parseNodeReference(xml));
	  return retval->getId();
	}
  };

  class PlexilTimepointVarParser: public PlexilInternalVarParser {
  public:
	PlexilTimepointVarParser() : PlexilInternalVarParser() {}
	~PlexilTimepointVarParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilTimepointVar* retval = new PlexilTimepointVar();
	  retval->setRef(parseNodeReference(xml));

	  const TiXmlElement* state = xml->FirstChildElement(STATEVAL_TAG);
	  checkParserExceptionWithLocation(state != NULL,
									   xml,
									   "XML parsing error: Timepoint missing " << STATEVAL_TAG << " tag");
	  checkNotEmpty(state);
	  retval->setState(state->FirstChild()->Value());

	  const TiXmlElement* point = xml->FirstChildElement(TIMEPOINT_TAG);
	  checkParserExceptionWithLocation(point != NULL,
									   xml,
									   "XML parsing error: Timepoint missing " << TIMEPOINT_TAG << " tag");
	  checkNotEmpty(point);
	  retval->setTimepoint(point->FirstChild()->Value());

	  return retval->getId();
	}
  };

  class PlexilOpParser: public PlexilExprParser {
  public:
	PlexilOpParser() : PlexilExprParser() {}
	~PlexilOpParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) 
	{
	  PlexilOp* retval = new PlexilOp();
	  retval->setOp(xml->Value());
	  for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child
			 = child->NextSiblingElement())
		retval->addSubExpr(PlexilXmlParser::parseExpr(child));
	  return retval->getId();
	}
  };

  class PlexilChangeLookupParser: public PlexilExprParser {
  public:
	PlexilChangeLookupParser() : PlexilExprParser() {}
	~PlexilChangeLookupParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(LOOKUPCHANGE_TAG, xml);
	  PlexilChangeLookup* retval = new PlexilChangeLookup();
	  retval->setState(PlexilXmlParser::parseState(xml));
	  for (const TiXmlElement* tol = xml->FirstChildElement(TOLERANCE_TAG); tol
			 != NULL; tol = tol->NextSiblingElement(TOLERANCE_TAG)) {
		checkHasChildElement(tol);
		retval->addTolerance(PlexilXmlParser::parseExpr(
														tol->FirstChildElement()));
	  }
	  return retval->getId();
	}
  };

  class PlexilLookupNowParser: public PlexilExprParser {
  public:
	PlexilLookupNowParser() : PlexilExprParser() {}
	~PlexilLookupNowParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(LOOKUPNOW_TAG, xml);
	  PlexilLookupNow* retval = new PlexilLookupNow();
	  retval->setState(PlexilXmlParser::parseState(xml));
	  return retval->getId();
	}
  };

  class PlexilArrayElementParser: public PlexilExprParser {
  public:
	PlexilArrayElementParser() : PlexilExprParser() {}
	~PlexilArrayElementParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(ARRAYELEMENT_TAG, xml);

	  // create an array element

	  PlexilArrayElement* arrayElement = new PlexilArrayElement();

	  // extract array name

	  const TiXmlElement* child = xml->FirstChildElement();
	  checkTag(NAME_TAG, child);
	  std::string name = child->FirstChild()->Value();
	  arrayElement->setArrayName(name);

	  // extract index

	  child = child->NextSiblingElement();
	  checkTag(INDEX_TAG, child);
	  PlexilExprId indexExpr = PlexilXmlParser::parseExpr(
														  child->FirstChildElement());
	  arrayElement->addSubExpr(indexExpr);

	  // return new array element

	  return arrayElement->getId();
	}
  };

  class PlexilValueParser: public PlexilExprParser {
  public:
	PlexilValueParser() : PlexilExprParser() {}
	~PlexilValueParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  // confirm that we have a value

	  checkTagPart(VAL_TAG, xml);

	  // get value (which could be empty)

	  std::string tag;
	  if (xml->Value() != NULL)
		tag = xml->Value();

	  // establish value type

	  std::string type = tag.substr(0, tag.find(VAL_TAG));

	  // establish the value (could be empty)

	  std::string value;
	  if (xml->FirstChild() != NULL && xml->FirstChild()->Value() != NULL)
		value = xml->FirstChild()->Value();

	  // return new value
	  return (new PlexilValue(PlexilXmlParser::parseValueType(type), value))->getId();
	}
  };

  class PlexilArrayValueParser: public PlexilExprParser {
  public:
	PlexilArrayValueParser() : PlexilExprParser() {}
	~PlexilArrayValueParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  // confirm that we have an array value
	  checkTag(ARRAY_VAL_TAG, xml);

	  // confirm that we have an element type
	  checkAttr(TYPE_TAG, xml);
	  std::string valueType(xml->Attribute(TYPE_TAG.c_str()));

	  // gather elements
	  std::vector<std::string> values;

	  const TiXmlElement* thisElement = xml->FirstChildElement();
	  while (thisElement != NULL) {
		checkTagPart(VAL_TAG, thisElement);
		// Check type
		std::string thisElementTag(thisElement->Value());
		std::string thisElementType = thisElementTag.substr(0, thisElementTag.find(VAL_TAG));
		checkParserExceptionWithLocation(thisElementType == valueType,
										 thisElement,
										 "XML parsing error: Array element type '" << thisElementTag
										 << "' in array value of type '" << valueType << "'");

		// Get array element value
		const char* thisElementValue = thisElement->GetText();
		if (thisElementValue != NULL && strlen(thisElementValue) != 0) {
		  values.push_back(std::string(thisElementValue));
		}
		else if (valueType == STRING_TAG) {
		  values.push_back(std::string());
		}
		else {
		  // parse error - empty array element not of type string
		  checkParserExceptionWithLocation(ALWAYS_FAIL,
										   thisElement,
										   "XML parsing error: Empty element value in array value of type '" << valueType << "'");
		}
	  }

	  // return new value
	  return (new PlexilArrayValue(PlexilXmlParser::parseValueType(valueType),
								   values.size(),
								   values))->getId();
	}
  };

  class PlexilVarRefParser: public PlexilExprParser {
  public:
	PlexilVarRefParser() : PlexilExprParser() {}
	~PlexilVarRefParser() {}

	PlexilExprId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTagPart(VAR_TAG, xml);
	  checkNotEmpty(xml);
	  PlexilVarRef* retval = new PlexilVarRef();
	  retval->setName(xml->FirstChild()->Value());
	  const char* varStart;
	  if ((varStart = strstr(xml->Value(), VAR_TAG.c_str())) != xml->Value()) {
		std::string type(xml->Value(), varStart - xml->Value());
		retval->setType(PlexilParser::parseValueType(type));
	  }
	  return retval->getId();
	}
  };

  class PlexilActionParser: public PlexilBodyParser {
  public:
	PlexilActionParser() : PlexilBodyParser() {}
	virtual ~PlexilActionParser() {}

	void parseDest(const TiXmlElement* xml, PlexilActionBody* body)
	  throw(ParserException) {
	  for (const TiXmlElement* var = xml->FirstChildElement(); var != NULL; var
			 = var->NextSiblingElement()) {
		std::string tag(var->Value());
		if (tag.find(VAR_TAG) != std::string::npos) {
		  body->addDestVar(PlexilVarRefParser().parse(var));
		} else if (tag.find(ARRAYELEMENT_TAG) != std::string::npos) {
		  body->addDestVar(PlexilArrayElementParser().parse(var));
		}
		continue;
	  }
	}
  };

  class PlexilAssignmentParser: public PlexilActionParser {
  public:
	PlexilAssignmentParser() : PlexilActionParser() {}
	~PlexilAssignmentParser() {}

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(ASSN_TAG, xml);
	  PlexilAssignmentBody* retval = new PlexilAssignmentBody();
	  parseDest(xml, retval);
	  const TiXmlElement* rhs = NULL;
	  for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child
			 = child->NextSiblingElement()) {
		std::string tag(child->Value());
		std::string::size_type pos = tag.find(RHS_TAG);
		if (pos != std::string::npos) {
		  // *** N.B. Used to try to get expression type info here,
		  // but that info is ambiguous and not used at present.
		  // So ignore it.
		  rhs = child;
		  break;
		}
	  }

	  checkParserExceptionWithLocation(rhs != NULL,
									   xml,
									   "XML parsing error: Missing RHS (return value) tags for " << xml->Value());
	  checkParserExceptionWithLocation(rhs->FirstChildElement() != NULL,
									   rhs,
									   "XML parsing error: Empty RHS (return value) tags for " << xml->Value());
	  retval->setRHS(PlexilXmlParser::parseExpr(rhs->FirstChildElement()));
	  return retval->getId();
	}
  };

  class PlexilNodeListParser: public PlexilBodyParser {
  public:
	PlexilNodeListParser() : PlexilBodyParser() {}
	~PlexilNodeListParser() {}

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(NODELIST_TAG, xml);
	  PlexilListBody* retval = new PlexilListBody();
	  std::set<std::string> childIds;
	  for (const TiXmlElement* child = xml->FirstChildElement(NODE_TAG); 
		   child != NULL;
		   child = child->NextSiblingElement(NODE_TAG)) {
		PlexilNodeId thisNode = PlexilXmlParser::parseNode(child);
		// check for duplicate node ID
		checkParserExceptionWithLocation(childIds.find(thisNode->nodeId()) == childIds.end(),
										 child,
										 "XML parsing error: Multiple nodes with node ID \"" << thisNode->nodeId() << "\"");
		childIds.insert(thisNode->nodeId());
		retval->addChild(thisNode);
	  }
	  return retval->getId();
	}
  };
  // parse a library node call

  class PlexilLibraryNodeCallParser: public PlexilBodyParser {
  public:
	PlexilLibraryNodeCallParser() : PlexilBodyParser() {}
	~PlexilLibraryNodeCallParser() {}

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(LIBRARYNODECALL_TAG, xml);

	  // get node id

	  const TiXmlElement* nodeIdXml = xml->FirstChildElement(NODEID_TAG);
	  checkParserExceptionWithLocation(nodeIdXml != NULL,
									   xml,
									   "XML parsing error: Missing <NodeId> element in library call.");
	  std::string nodeId(nodeIdXml->FirstChild()->Value());
	  checkParserExceptionWithLocation(!nodeId.empty(),
									   nodeIdXml,
									   "XML parsing error: Empty <NodeId> element in library call.");

	  // create lib node call node body

	  PlexilLibNodeCallBody* body = new PlexilLibNodeCallBody(nodeId);

	  // collect the variable alias information

	  for (const TiXmlElement* child = xml->FirstChildElement(ALIAS_TAG); child
			 != NULL; child = child->NextSiblingElement(ALIAS_TAG)) {
		// get library node parameter

		const TiXmlElement* libParamXml = child->FirstChildElement(NODE_PARAMETER_TAG);
		checkParserExceptionWithLocation(libParamXml != NULL,
										 child,
										 "XML parsing library error: Missing <NodeParameter> element in library call.");
		std::string libParam(libParamXml->FirstChild()->Value());
		checkParserExceptionWithLocation(!libParam.empty(),
										 libParamXml,
										 "XML parsing library error: Empty <NodeParameter> element in library call.");

		// get node parameter value

		PlexilExprId value = PlexilXmlParser::parseExpr(libParamXml->NextSiblingElement());

		// add alias to body

		body->addAlias(libParam, value);
	  }
	  // return lib node call node body

	  return body->getId();
	}
  };

  class PlexilCommandParser: public PlexilActionParser {
  public:
	PlexilCommandParser() : PlexilActionParser() {}
	~PlexilCommandParser() {}

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(CMD_TAG, xml);
	  PlexilCommandBody* retval = new PlexilCommandBody();
	  parseDest(xml, retval);
	  retval->setState(PlexilXmlParser::parseState(xml));
	  retval->setResource(PlexilXmlParser::parseResource(xml));
	  return retval->getId();
	}
  };

  class PlexilPairsParser: public PlexilBodyParser {
  public:
	PlexilPairsParser() : PlexilBodyParser() {}
	virtual ~PlexilPairsParser() {}

	PlexilUpdateId parsePairs(const TiXmlElement* xml) throw(ParserException) {
	  PlexilUpdateId retval = (new PlexilUpdate())->getId();
	  for (const TiXmlElement* child = xml->FirstChildElement(PAIR_TAG); child
			 != NULL; child = child->NextSiblingElement(PAIR_TAG)) {
		checkNotEmpty(child->FirstChildElement(NAME_TAG));
		std::string name =
		  child->FirstChildElement(NAME_TAG)->FirstChild()->Value();
		const TiXmlElement* value = child->FirstChildElement();
		while (value != NULL && value->Value() == NAME_TAG)
		  value = value->NextSiblingElement();
		checkParserExceptionWithLocation(value != NULL,
										 child,
										 "XML parsing error: No update value in pair for variable '" << name << "'");
		debugMsg("PlexilXml:parsePairs", "Parsed pair {" << name << ", " << *value << "}");
		retval->addPair(name, PlexilXmlParser::parseExpr(value));
	  }
	  return retval->getId();
	}
  };

  class PlexilUpdateParser: public PlexilPairsParser {
  public:
	PlexilUpdateParser() : PlexilPairsParser() {}
    ~PlexilUpdateParser() {} 

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(UPDATE_TAG, xml);
	  PlexilUpdateBody* retval = new PlexilUpdateBody();
	  if (xml->FirstChildElement(PAIR_TAG) != NULL)
		retval->setUpdate(parsePairs(xml));
	  return retval->getId();
	}
  };

  class PlexilRequestParser: public PlexilPairsParser {
  public:
	PlexilRequestParser() : PlexilPairsParser() {}
    ~PlexilRequestParser() {} 

	PlexilNodeBodyId parse(const TiXmlElement* xml) throw(ParserException) {
	  checkTag(REQ_TAG, xml);
	  PlexilRequestBody* retval = new PlexilRequestBody();
	  if (xml->FirstChildElement(PAIR_TAG) != NULL)
		retval->setUpdate(parsePairs(xml));
	  const TiXmlElement* ref = xml->FirstChildElement(NODEREF_TAG);
	  if (ref != NULL)
		retval->setParent(PlexilXmlParser::parseNodeRef(ref));
	  return retval->getId();
	}
  };

  void PlexilXmlParser::registerParsers() 
  {
	if (s_init) {
	  s_bodyParsers = new std::map<std::string, PlexilBodyParser*>();
	  s_bodyParsers->insert(std::make_pair(ASSN_TAG, new PlexilAssignmentParser()));
	  s_bodyParsers->insert(std::make_pair(NODELIST_TAG,
										   new PlexilNodeListParser()));
	  s_bodyParsers->insert(std::make_pair(LIBRARYNODECALL_TAG,
										   new PlexilLibraryNodeCallParser()));
	  s_bodyParsers->insert(std::make_pair(CMD_TAG, new PlexilCommandParser()));
	  s_bodyParsers->insert(std::make_pair(UPDATE_TAG, new PlexilUpdateParser()));
	  s_bodyParsers->insert(std::make_pair(REQ_TAG, new PlexilRequestParser()));

	  s_exprParsers = new std::map<std::string, PlexilExprParser*>();
	  PlexilExprParser* varRef = new PlexilVarRefParser();
	  s_exprParsers->insert(std::make_pair(VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(INT_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(REAL_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(ARRAY_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(STRING_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(BOOL_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(TIME_TAG + VAR_TAG, varRef));
	  s_exprParsers->insert(std::make_pair(BLOB_TAG + VAR_TAG, varRef));

	  s_exprParsers->insert(std::make_pair(NODE_OUTCOME_TAG + VAR_TAG,
										   new PlexilOutcomeVarParser()));
	  s_exprParsers->insert(std::make_pair(NODE_FAILURE_TAG + VAR_TAG,
										   new PlexilFailureVarParser()));
	  s_exprParsers->insert(std::make_pair(NODE_STATE_TAG + VAR_TAG,
										   new PlexilStateVarParser()));
	  s_exprParsers->insert(std::make_pair(NODE_COMMAND_HANDLE_TAG + VAR_TAG,
										   new PlexilCommandHandleVarParser()));
	  s_exprParsers->insert(std::make_pair("NodeTimepoint" + VAL_TAG,
										   new PlexilTimepointVarParser()));

	  PlexilExprParser* val = new PlexilValueParser();
	  s_exprParsers->insert(std::make_pair(INT_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(REAL_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(STRING_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(BOOL_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(TIME_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(BLOB_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(NODE_OUTCOME_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(NODE_FAILURE_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(NODE_STATE_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(NODE_COMMAND_HANDLE_TAG + VAL_TAG, val));
	  s_exprParsers->insert(std::make_pair(LOOKUPNOW_TAG,
										   new PlexilLookupNowParser()));
	  s_exprParsers->insert(std::make_pair(LOOKUPCHANGE_TAG,
										   new PlexilChangeLookupParser()));
	  s_exprParsers->insert(std::make_pair(ARRAYELEMENT_TAG,
										   new PlexilArrayElementParser()));

	  PlexilExprParser* op = new PlexilOpParser();
	  s_exprParsers->insert(std::make_pair("AND", op));
	  s_exprParsers->insert(std::make_pair("OR", op));
	  s_exprParsers->insert(std::make_pair("XOR", op));
	  s_exprParsers->insert(std::make_pair("NOT", op));
	  s_exprParsers->insert(std::make_pair("Concat", op));
	  s_exprParsers->insert(std::make_pair("IsKnown", op));
	  s_exprParsers->insert(std::make_pair("EQ", op));
	  s_exprParsers->insert(std::make_pair("EQNumeric", op));
	  s_exprParsers->insert(std::make_pair("EQString", op));
	  s_exprParsers->insert(std::make_pair("EQBoolean", op));
	  s_exprParsers->insert(std::make_pair("EQInternal", op));
	  s_exprParsers->insert(std::make_pair("NE", op));
	  s_exprParsers->insert(std::make_pair("NENumeric", op));
	  s_exprParsers->insert(std::make_pair("NEString", op));
	  s_exprParsers->insert(std::make_pair("NEBoolean", op));
	  s_exprParsers->insert(std::make_pair("NEInternal", op));
	  s_exprParsers->insert(std::make_pair("LT", op));
	  s_exprParsers->insert(std::make_pair("LE", op));
	  s_exprParsers->insert(std::make_pair("GT", op));
	  s_exprParsers->insert(std::make_pair("GE", op));
	  s_exprParsers->insert(std::make_pair("ADD", op));
	  s_exprParsers->insert(std::make_pair("SUB", op));
	  s_exprParsers->insert(std::make_pair("MUL", op));
	  s_exprParsers->insert(std::make_pair("DIV", op));
	  s_exprParsers->insert(std::make_pair("MOD", op));
	  s_exprParsers->insert(std::make_pair("SQRT", op));
	  s_exprParsers->insert(std::make_pair("ABS", op));

	  s_init = false;
	}
  }

  /*
   * @brief Load the named library node from a file on the given path.
   * @param name Name of the node.
   * @param path Vector of places to search for the file.
   * @return The loaded node, or noId() if not found or error.
   */
  PlexilNodeId PlexilXmlParser::findLibraryNode(const std::string& name,
												const std::vector<std::string>& path)
  {
	return findPlan(name, name, path);
  }

  /*
   * @brief Load the named plan from a file on the given path.
   * @param name Name of the node.
   * @param fileName Name of the file, with or without the ".plx" suffix.
   * @param path Vector of places to search for the file.
   * @return The loaded node, or noId() if not found or error.
   */
  PlexilNodeId PlexilXmlParser::findPlan(const std::string& name,
										 const std::string& fileName,
										 const std::vector<std::string>& path)
  {
	PlexilNodeId result;
	std::vector<std::string>::const_iterator it = path.begin();
	std::string fileNameWithSuffix = fileName;
	// TODO: add suffix if lacking
	if (fileNameWithSuffix.compare(fileNameWithSuffix.length() - 4, 4, ".plx") != 0)
	  fileNameWithSuffix += ".plx";

	// Find the first occurrence of the library in this path
	while (result.isNoId() && it != path.end()) {
	  std::string candidateFile = *it + "/" + fileNameWithSuffix;
		try {
		  result = loadPlanNamed(name, candidateFile);
		  if (result.isId())
			return result;
		}
		catch (ParserException& p) {
		  debugMsg("PlexilXmlParser:findPlan", 
				   " failed due to error in " 
				   << candidateFile
				   << ":\n"
				   << p.what());
		  return PlexilNodeId::noId();
		}
		it++;
	  }

	// check current working directory
	try {
	  return loadPlanNamed(name, fileNameWithSuffix);
	}
	catch (ParserException& p) {
	  debugMsg("PlexilXmlParser:findLibraryNode", 
			   " failed due to error in " 
			   << fileNameWithSuffix
			   << ":\n"
			   << p.what());
	  return PlexilNodeId::noId();
	}
  }

  /*
   * @brief Load the named plan from a file in the given directory.
   * @param name Name of the desired node.
   * @param filename Candidate file for this node.
   * @return The loaded node, or noId() if not found or error.
   */
  PlexilNodeId PlexilXmlParser::loadPlanNamed(const std::string& name, 
											  const std::string& filename)
	  throw(ParserException)
  {
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) {
	  checkParserException(doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE,
						   "Error reading XML file " << filename
						   << ": " << doc.ErrorDesc());
	  debugMsg("PlexilXmlParser:loadPlanNamed", 
			   " unable to open file " << filename);
	  return PlexilNodeId::noId();
	}
	PlexilNodeId result = parse(doc.RootElement());
	// Check that node has the desired node ID
	checkParserException(0 == name.compare(result->nodeId()),
						 "Error: File " << filename
						 << " contains node ID \"" << result->nodeId()
						 << "\", not \"" << name << "\"");
	debugMsg("PlexilXmlParser:loadPlanNamed",
			 " successfully loaded node " << name << " from " << filename);
	return result;
  }

  PlexilNodeId PlexilXmlParser::parse(const std::string& str, bool isFile)
	throw(ParserException) 
  {
	if (!isFile)
	  return parse(str.c_str());

	registerParsers();
	TiXmlDocument doc(str);
	checkParserException(doc.LoadFile(),
						 "Error reading XML file " << str << ": " << doc.ErrorDesc());
	TiXmlElement* root = doc.RootElement();
	checkParserException(root != NULL, "No root node in file " << str);
	PlexilNodeId result = parse(root);
	if (result->fileName().empty())
	  result->setFileName(str);
	return result;
  }

  PlexilNodeId PlexilXmlParser::parse(const char* text)
	throw(ParserException) 
  {
	registerParsers();

	// First parse the XML itself
	TiXmlDocument doc;
	doc.LoadFile(text);
	checkParserException(!doc.Error(),
						 "(line " << doc.ErrorRow()
						 << ", column " << doc.ErrorCol()
						 << ") XML parsing error: " << doc.ErrorDesc());

	TiXmlElement* root = doc.RootElement();
	PlexilNodeId result = parseNode(root);
	return result;
  }

  PlexilNodeId PlexilXmlParser::parse(const TiXmlElement* xml)
	throw(ParserException) 
  {
	registerParsers();
	// strip away PlexilPlan wrapper, if any
	if (0 == PLEXIL_PLAN_TAG.compare(xml->Value())) {
	  const TiXmlElement* node = xml->FirstChildElement(NODE_TAG);
	  checkParserException(node != NULL, "No root node in " << xml);
	  xml = node;
	}
	return parseNode(xml);
  }

  PlexilExprId PlexilXmlParser::parseExpr(const TiXmlElement* xml)
	throw(ParserException) {
	std::map<std::string, PlexilExprParser*>::iterator it = s_exprParsers->find(
																				xml->Value());
	checkParserExceptionWithLocation(it != s_exprParsers->end(),
									 xml,
									 "XML parsing error: No parser for expression '" << xml->Value() << "'");
	return it->second->parse(xml);
  }

  PlexilNodeId PlexilXmlParser::parseNode(const TiXmlElement* xml)
	throw(ParserException) {
	checkTag(NODE_TAG, xml);
	PlexilNodeId retval = (new PlexilNode())->getId();

	// nodeid required

	const TiXmlElement* nodeIdXml = xml->FirstChildElement(NODEID_TAG);
	checkParserExceptionWithLocation(nodeIdXml != NULL,
									 xml,
									 "XML parsing error: Missing or empty <NodeId> element.");
	std::string nodeId(nodeIdXml->FirstChild()->Value());
	checkParserExceptionWithLocation(!nodeId.empty(),
									 nodeIdXml,
									 "XML parsing error: Missing or empty <NodeId> element.");
	retval->setNodeId(nodeIdXml->FirstChild()->Value());

	// node type required

	checkAttr(NODETYPE_ATTR, xml);
	retval->setNodeType(*(xml->Attribute(NODETYPE_ATTR)));

	// file name, line, col optional
	const std::string* fname = xml->Attribute(FILENAME_ATTR);
	if (fname != NULL)
	  retval->setFileName(fname->c_str());
	int line = 0;
	xml->Attribute(LINENO_ATTR, &line);
	if (line != 0)
	  retval->setLineNo(line);
	int col = 0;
	xml->Attribute(COLNO_ATTR, &col);
	if (col != 0)
	  retval->setColNo(col);

	// priority optional

	const TiXmlElement* priorityXml = xml->FirstChildElement(PRIORITY_TAG);
	if (priorityXml != NULL) {
	  std::string priority = priorityXml->FirstChild()->Value();
	  if (!priority.empty()) {
		std::stringstream str;
		double value;
		str << priority;
		str >> value;
		retval->setPriority(value);
	  }
	}

	// permissions optional

	const TiXmlElement* permissionsXml = xml->FirstChildElement(PERMISSIONS_TAG);
	if (permissionsXml != NULL)
	  retval->setPermissions(permissionsXml->FirstChild()->Value());

	// interface optional

	const TiXmlElement* interfaceXml = xml->FirstChildElement(INTERFACE_TAG);
	if (interfaceXml != NULL)
	  retval->setInterface(parseInterface(interfaceXml));

	// variable declarations optional

	const TiXmlElement* declarationsXml = xml->FirstChildElement(VAR_DECLS_TAG);
	if (declarationsXml != NULL)
	  parseDeclarations(declarationsXml, retval);

	// conditions optional

	for (const TiXmlElement* conditionsXml = xml->FirstChildElement(); conditionsXml
		   != NULL; conditionsXml = conditionsXml->NextSiblingElement()) {
	  std::string tag = conditionsXml->Value();
	  if (tag.find(COND_TAG) == std::string::npos)
		continue;
	  retval->addCondition(tag, parseExpr(conditionsXml->FirstChildElement()));
	}

	// node body optional

	const TiXmlElement* bodyXml = xml->FirstChildElement(BODY_TAG);
	if (bodyXml == NULL) {
	  checkParserExceptionWithLocation(retval->nodeType() == NodeType_Empty,
									   xml,
									   "XML parsing error: " << retval->nodeTypeString() << " node '" << retval->nodeId() <<
									   "' missing <NodeBody> element. '" << retval->nodeTypeString() <<
									   "' nodes must contain a '" << retval->nodeTypeString() <<
									   "' as a <NodeBody> element.");
	}
	else {
	  const TiXmlElement* realBodyXml = bodyXml->FirstChildElement();
	  if (realBodyXml != NULL) {
		retval->setBody(parseBody(realBodyXml));
	  }
	}

	return retval;
  }

  PlexilInterfaceId PlexilXmlParser::parseDeprecatedInterface(
															  const TiXmlElement* intf) throw(ParserException) {
	checkTag(INTERFACE_TAG, intf);
	PlexilInterfaceId retval = (new PlexilInterface())->getId();
	const TiXmlElement* in = intf->FirstChildElement(IN_TAG);
	PlexilVarRefParser p;
	if (in != NULL)
	  for (const TiXmlElement* var = in->FirstChildElement(); var != NULL; var
			 = var->NextSiblingElement())
		retval->addIn(p.parse(var));
	const TiXmlElement* inOut = intf->FirstChildElement(INOUT_TAG);
	if (inOut != NULL)
	  for (const TiXmlElement* var = inOut->FirstChildElement(); var != NULL; var
			 = var->NextSiblingElement())
		retval->addInOut(p.parse(var));
	return retval;
  }

  PlexilInterfaceId PlexilXmlParser::parseInterface(const TiXmlElement* intf)
	throw(ParserException) {
	PlexilInterfaceId retval = (new PlexilInterface())->getId();
	checkTag(INTERFACE_TAG, intf);
	parseInOrInOut(intf->FirstChildElement(IN_TAG), retval, false);
	parseInOrInOut(intf->FirstChildElement(INOUT_TAG), retval, true);
	return retval;
  }

  void PlexilXmlParser::parseInOrInOut(const TiXmlElement* inOrInOut,
									   PlexilInterfaceId& interface, 
									   bool isInOut)
	throw(ParserException) 
  {
	// if this is an empty In or InOut section, just return
	if (inOrInOut == NULL)
	  return;

	for (const TiXmlElement* var = inOrInOut->FirstChildElement(); 
		 var != NULL; 
		 var = var->NextSiblingElement()) {

	  // if this is a declare var or array read those in
	  if (testTag(DECL_VAR_TAG, var) || testTag(DECL_ARRAY_TAG, var)) {
		PlexilVarId variable = parseDeclaration(var)->getId();

		// convert variable to var ref
		Id<PlexilVarRef> varRef = (new PlexilVarRef())->getId();
		varRef->setVariable(variable);

		// add var ref to interface
		if (isInOut)
		  interface->addInOut(varRef);
		else
		  interface->addIn(varRef);
	  }

	  // else this is the deprecated case
	  else {
		checkParserExceptionWithLocation(ALWAYS_FAIL,
										 var,
										 ") XML parsing error: Deprecated interface declaration syntax no longer supported");
	  
		warn("DEPRECATED: <" << var->Value() <<
			 "> tag, use <" << DECL_VAR_TAG <<
			 "> or <" << DECL_ARRAY_TAG <<
			 "> tag instead.");

		PlexilVarRefParser p;
		if (isInOut)
		  interface->addInOut(p.parse(var));
		else
		  interface->addIn(p.parse(var));
	  }
	}
  }

  void PlexilXmlParser::parseDeclarations(const TiXmlElement* decls,
										  PlexilNodeId &node) throw(ParserException) {
	checkTag(VAR_DECLS_TAG, decls);
	for (const TiXmlElement* decl = decls->FirstChildElement(); decl != NULL; decl
		   = decl->NextSiblingElement())
	  node->addVariable(parseDeclaration(decl)->getId());
  }

  PlexilVar* PlexilXmlParser::parseDeclaration(const TiXmlElement* decl)
	throw(ParserException) {
	// if array declaration

	if (testTag(DECL_ARRAY_TAG, decl))
	  return parseArrayDeclaration(decl);

	// if new vairaible declaration xml syntax

	if (testTag(DECL_VAR_TAG, decl))
	  return parseAtomicOrStringDeclaration(decl);

	// else its a deprecated variable declaration xml syntax

	return parseDeprecatedDeclaration(decl);
  }

  // parse an array declaration

  PlexilVar* PlexilXmlParser::parseArrayDeclaration(const TiXmlElement* decl)
	throw(ParserException) {
	checkTag(DECL_ARRAY_TAG, decl);

	// extract array name

	const TiXmlElement* child = decl->FirstChildElement();
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

	debugMsg("PlexilXmlParser:parseArrayDeclaration",
			 " for array " << name << ", element type " << type << ", size " << maxSize);

	// if present, extract initial values

	std::vector<std::string> initVals;
	if ((child = child->NextSiblingElement()) != NULL) {
	  checkTag(INITIALVAL_TAG, child);
	  child = child->FirstChildElement();
	  do {
		checkTagPart(VAL_TAG, child);
		std::string initValTag = std::string(child->Value());
		std::string initValType = initValTag.substr(0, initValTag.size() - VAL_TAG.size());
		checkParserExceptionWithLocation(type == initValType,
										 child,
										 "XML parsing error: Initial value of " << type << " array variable \'" <<
										 name << "\' of incorrect type \'" << initValType << "\'");
		std::string initVal = child->FirstChild()->Value();
		initVals.push_back(initVal);
		debugMsg("PlexilXmlParser:parseArrayDeclaration",
				 " element value \"" << initVal << "\"");
		checkParserExceptionWithLocation(initVals.size() <= maxSize,
										 child->FirstChild(),
										 "XML parsing error: Number of initial values of " << type <<
										 " array variable \'" << name <<
										 "\' exceeds maximum of " << maxSize);
	  } while ((child = child->NextSiblingElement()) != NULL);
	}

	// add variable to returned variable set

	PlexilVar* result =
	  new PlexilArrayVar(name, 
						 PlexilParser::parseValueType(type),
						 maxSize,
						 initVals);
	debugMsg("PlexilXmlParser:parseArrayDeclaration", " succeeded");
	return result;
  }

  // parse an atomic or string declaration

  PlexilVar* PlexilXmlParser::parseAtomicOrStringDeclaration(
															 const TiXmlElement* decl) throw(ParserException) {
	checkTag(DECL_VAR_TAG, decl);

	// extract name

	const TiXmlElement* child = decl->FirstChildElement();
	checkTag(NAME_TAG, child);
	std::string name = child->FirstChild()->Value();

	// extract type

	child = child->NextSiblingElement();
	checkTag(TYPE_TAG, child);
	std::string type = child->FirstChild()->Value();

	// if present, create variable with initial value

	if ((child = child->NextSiblingElement()) != NULL) {
	  checkTag(INITIALVAL_TAG, child);
	  child = child->FirstChildElement();
	  checkTagPart(VAL_TAG, child);
	  std::string initValTag = std::string(child->Value());
	  std::string initValType = initValTag .substr(0, initValTag.size()
												   - VAL_TAG.size());
	  checkParserExceptionWithLocation(type == initValType,
									   child,
									   "XML parsing error: Initial value of " << type << " variable \'" <<
									   name << "\' of incorrect type \'" <<
									   initValType << "\'");
	  return new PlexilVar(name, PlexilParser::parseValueType(type),
						   child->FirstChild()->Value());
	}

	// otherwise create variable with the value unknown

	return new PlexilVar(name, PlexilParser::parseValueType(type));
  }

  // parse a deprecated declaration

  PlexilVar* PlexilXmlParser::parseDeprecatedDeclaration(const TiXmlElement* decl)
	throw(ParserException) {
	checkTagPart(DECL_TAG, decl);
	std::string tag(decl->Value());
	PlexilType type = parseValueType(tag.substr(DECL_TAG.size()));
	std::string name;
	std::string value;

	const TiXmlElement* child = decl->FirstChildElement();
	getNameOrValue(child, name, value);
	child = child->NextSiblingElement();
	getNameOrValue(child, name, value);

	warn("DEPRECATED: <" << decl->Value() <<
		 "> tag, use <DeclareVariable> tag instead.");

	checkParserExceptionWithLocation(!name.empty(),
									 child,
									 "XML parsing error: Must have a tag ending in '" <<
									 VAR_TAG << "' as a child of a <" <<
									 VAR_DECLS_TAG << "> element.");
	if (value.empty())
	  return new PlexilVar(name, type);
	else
	  return new PlexilVar(name, type, value);
  }

  PlexilNodeBodyId PlexilXmlParser::parseBody(const TiXmlElement* body)
	throw(ParserException) {
	std::string name(body->Value());
	std::map<std::string, PlexilBodyParser*>::iterator it = s_bodyParsers->find(
																				name);
	checkParserExceptionWithLocation(it != s_bodyParsers->end(),
									 body,
									 "XML parsing error: No parser for body type " << name);
	return it->second->parse(body);
  }

  PlexilStateId PlexilXmlParser::parseState(const TiXmlElement* xml)
	throw(ParserException) 
  {
	PlexilStateId retval = (new PlexilState())->getId();
	const TiXmlElement* arguments = NULL;
	for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child
		   = child->NextSiblingElement()) 
	  {
		// get tag string
		const std::string& tag(child->ValueStr());

		// if name, it should contain a string variable or value expression

		if (tag == NAME_TAG) 
		  {
			debugMsg("PlexilXmlParser::parseState", " name expression = " << *child);
			checkHasChildElement(child);
			retval->setNameExpr(parseExpr(child->FirstChildElement()));
		  }

		// if it's an argument use that
		else if (tag == ARGS_TAG)
		  {
			debugMsg("PlexilXmlParser::parseState", " args = " << *child);
			arguments = child;
		  }
	  }

	if (arguments != NULL) {
	  for (const TiXmlElement* child = arguments->FirstChildElement();
		   child != NULL;
		   child = child->NextSiblingElement()) 
		{
		  retval->addArg(PlexilXmlParser::parseExpr(child));
		}
	}

	return retval;
  }

  std::vector<PlexilResourceId> 
  PlexilXmlParser::parseResource(const TiXmlElement* xml) 
	throw(ParserException) 
  {
	// Redundant with caller's check
	// checkTag(CMD_TAG, xml);

	// Create a vector of PlexilResourceId
	std::vector<PlexilResourceId> rId_vec;

	for (const TiXmlElement* child = xml->FirstChildElement();
		 child != NULL; 
		 child = child->NextSiblingElement()) {
	  std::string tag(child->Value());
	  // Parse only resource list tag
	  if (tag == RESOURCELIST_TAG) {
		// Loop through each resource in the list
		for (const TiXmlElement* resourceElt = child->FirstChildElement(); 
			 resourceElt != NULL;
			 resourceElt = resourceElt->NextSiblingElement()) {

		  checkTag(RESOURCE_TAG, resourceElt);

		  // check that the resource has a name and a priority
		  checkParserExceptionWithLocation(resourceElt->FirstChildElement(RESOURCENAME_TAG) != NULL,
										   resourceElt,
										   "XML parsing error: No " << RESOURCENAME_TAG << " element for resource");
		  checkParserExceptionWithLocation(resourceElt->FirstChildElement(RESOURCEPRIORITY_TAG) != NULL,
										   resourceElt,
										   "XML parsing error: No " << RESOURCEPRIORITY_TAG << " element for resource");

		  // Create a new PlexilResourceId.
		  PlexilResourceId prId = (new PlexilResource())->getId();
		  // loop through each resource element
		  for (const TiXmlElement* child3 = resourceElt->FirstChildElement(); 
			   child3 != NULL; 
			   child3 = child3->NextSiblingElement()) {
			// add each resource element just like addArg to PLexilResourceId. Use
			// tag3 and expresssion the in <name, expr> pair
			prId->addResource(child3->Value(),
							  PlexilXmlParser::parseExpr(child3->FirstChildElement()));
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
	throw(ParserException) {
	checkTag(NODEREF_TAG, ref);
	checkAttr(DIR_ATTR, ref);
	PlexilNodeRefId retval = (new PlexilNodeRef())->getId();
	if (*(ref->Attribute(DIR_ATTR)) == CHILD_VAL)
	  retval->setDir(PlexilNodeRef::CHILD);
	else if (*(ref->Attribute(DIR_ATTR)) == PARENT_VAL)
	  retval->setDir(PlexilNodeRef::PARENT);
	else if (*(ref->Attribute(DIR_ATTR)) == SIBLING_VAL)
	  retval->setDir(PlexilNodeRef::SIBLING);
	else if (*(ref->Attribute(DIR_ATTR)) == SELF_VAL)
	  retval->setDir(PlexilNodeRef::SELF);
	else {
	  checkParserExceptionWithLocation(ALWAYS_FAIL,
									   ref,
									   "XML parsing error: Invalid value for 'dir' attibute: " << *(ref->Attribute(DIR_ATTR)));
	}
	if (retval->dir() != PlexilNodeRef::PARENT && retval->dir()
		!= PlexilNodeRef::SELF) {
	  checkNotEmpty(ref);
	  retval->setName(ref->FirstChild()->Value());
	}
	return retval;
  }

  void PlexilXmlParser::getNameOrValue(const TiXmlElement* xml,
									   std::string& name, std::string& value) {
	if (xml == NULL)
	  return;
	std::string tag(xml->Value());
	if (tag.find(VAR_TAG) != std::string::npos)
	  name = xml->FirstChild()->Value();
	else if (tag.find(VAL_TAG) != std::string::npos) {
	  if (!xml->NoChildren())
		value = xml->FirstChild()->Value();
	}
  }

  TiXmlElement* PlexilXmlParser::getNodeParent(const TiXmlElement* node) {
	if (node->Parent() == NULL)
	  return NULL;
	if (node->Parent()->Value() == NODE_TAG)
	  return (TiXmlElement*) node->Parent();
	return getNodeParent((TiXmlElement*) node->Parent());
  }

  PlexilNodeRefId PlexilXmlParser::getNodeRef(const TiXmlElement* ref,
											  const TiXmlElement* node)
	throw(ParserException) {
	checkTag(NODEID_TAG, ref);
	std::string name(ref->FirstChild()->Value());

	debugMsg("PlexilXmlParser:getNodeRef", " for \"" << name << "\"");

	const TiXmlElement
	  *checkSelf = NULL,
	  *checkParent = NULL,
	  *checkSibling = NULL,
	  *checkChild = NULL;
	checkTag(NODE_TAG, node);

	const TiXmlElement* selfId = node->FirstChildElement(NODEID_TAG);
	if (selfId != NULL
		&& selfId->FirstChild() != NULL
		&& name == selfId->FirstChild()->Value()) {
	  debugMsg("PlexilXmlParser:getNodeRef",
			   "Found self with name " << name);
	  checkSelf = const_cast<TiXmlElement*> (node);
	}

	const TiXmlElement* parent = getNodeParent(node);
	if (parent != NULL) {

	  //find parent with name
	  const TiXmlElement* parentId = parent->FirstChildElement(NODEID_TAG);
	  if (parentId != NULL && parentId->FirstChild() != NULL
		  && parentId->FirstChild()->Value() == name) {
		checkParent = (const TiXmlElement*) node->Parent();
		debugMsg("PlexilXmlParser:getNodeRef",
				 "Found parent with name " << name);
	  }

	  //get siblings with name
	  for (checkSibling =
			 parent->FirstChildElement(BODY_TAG)->FirstChildElement(NODELIST_TAG)
			 ->FirstChildElement(NODE_TAG); 
		   checkSibling != NULL;
		   checkSibling = checkSibling->NextSiblingElement(NODE_TAG)) {
		const TiXmlElement* siblingId = checkSibling->FirstChildElement(NODEID_TAG);
		if (checkSibling != checkSelf && siblingId != NULL
			&& siblingId->FirstChild() != NULL
			&& siblingId->FirstChild()->Value() == name) {
		  debugMsg("PlexilXmlParser:getNodeRef",
				   "Found sibling with name " << name);
		  break;
		}
	  }
	}

	//get children with name
	if (node->FirstChildElement(BODY_TAG) != NULL
		&& node->FirstChildElement(BODY_TAG)->FirstChildElement(NODELIST_TAG) != NULL) {
	  for (checkChild = 
			 node->FirstChildElement(BODY_TAG)->FirstChildElement(NODELIST_TAG)
			 ->FirstChildElement(NODE_TAG);
		   checkChild != NULL;
		   checkChild = checkChild->NextSiblingElement(NODE_TAG)) {
		const TiXmlElement* childId = checkChild->FirstChildElement(NODEID_TAG);
		if (childId != NULL && childId->FirstChild() != NULL
			&& childId->FirstChild()->Value() == name) {
		  debugMsg("PlexilXmlParser:getNodeRef",
				   "Found child with name " << name);
		  break;
		}
	  }
	}

	PlexilNodeRefId retval;
	if (checkSelf != NULL) {
	  retval = (new PlexilNodeRef())->getId();
	  retval->setDir(PlexilNodeRef::SELF);
	}
	if (checkParent != NULL) {
	  checkParserExceptionWithLocation(!retval.isValid(),
									   ref->FirstChild(),
									   "XML parsing error: Ambiguous old-style node reference.\n Node "
									   << selfId->FirstChild()->Value()
									   << " and its parent are both named '" << name << "'");
	  retval = (new PlexilNodeRef())->getId();
	  retval->setDir(PlexilNodeRef::PARENT);
	}
	if (checkSibling != NULL) {
	  checkParserExceptionWithLocation(!retval.isValid(),
									   ref->FirstChild(),
									   "XML parsing error: Ambiguous old-style node reference.\n Node "
									   << selfId->FirstChild()->Value()
									   << " has a sibling and either a parent or itself named '" << name << "'");
	  retval = (new PlexilNodeRef())->getId();
	  retval->setDir(PlexilNodeRef::SIBLING);
	}
	if (checkChild != NULL) {
	  checkParserExceptionWithLocation(!retval.isValid(),
									   ref->FirstChild(),
									   "XML parsing error: Ambiguous old-style node reference.  Node "
									   << selfId->FirstChild()->Value() <<
									   " has a sibling, parent, or itself and a child named '" << name << "'");
	  retval = (new PlexilNodeRef())->getId();
	  retval->setDir(PlexilNodeRef::CHILD);
	}

	checkParserExceptionWithLocation(retval.isValid(),
									 ref->FirstChild(),
									 "XML parsing error: Node '" << selfId->FirstChild()->Value()
									 << "' is trying to access node '" << name
									 << "' which is out of scope or does not exist");
	retval->setName(name);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeId& node)
	throw(ParserException) {
	TiXmlElement* retval = new TiXmlElement(NODE_TAG);
	retval->SetAttribute(NODETYPE_ATTR, node->nodeTypeString());
	const std::string & filename = node->fileName();
	if (!filename.empty())
	  retval->SetAttribute(FILENAME_ATTR, filename);

	int lineno = node->lineNo();
	if (lineno != 0)
	  retval->SetAttribute(LINENO_ATTR, lineno);

	int col = node->colNo();
	if (col != 0)
	  retval->SetAttribute(COLNO_ATTR, col);

	retval->LinkEndChild(namedTextElement(NODEID_TAG, node->nodeId()));
	retval->LinkEndChild(namedTextElement(PERMISSIONS_TAG,
										  node->permissions()));
	retval->LinkEndChild(namedNumberElement(PRIORITY_TAG, node->priority()));

	if (node->interface().isValid())
	  retval->LinkEndChild(toXml(node->interface()));

	if (!node->declarations().empty()) {
	  TiXmlElement* declarations = element(VAR_DECLS_TAG);
	  for (std::vector<PlexilVarId>::const_iterator it =
			 node->declarations().begin(); it != node->declarations().end(); ++it)
		declarations->LinkEndChild(toXml(*it));
	  retval->LinkEndChild(declarations);
	}

	for (std::map<std::string, PlexilExprId>::const_iterator it =
		   node->conditions().begin(); it != node->conditions().end(); ++it) {
	  TiXmlElement* cond = element(it->first);
	  cond->LinkEndChild(toXml(it->second));
	  retval->LinkEndChild(cond);
	}

	if (node->body().isValid())
	  retval->LinkEndChild(toXml(node->body()));

	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilInterfaceId& intf)
	throw(ParserException) {
	TiXmlElement* retval = element(INTERFACE_TAG);
	if (!intf->in().empty()) {
	  TiXmlElement* in = element(IN_TAG);
	  for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin(); it
			 != intf->in().end(); ++it) {
		in->LinkEndChild(toXml(*it));
	  }
	  retval->LinkEndChild(in);
	}
	if (!intf->inOut().empty()) {
	  TiXmlElement* inOut = element(INOUT_TAG);
	  for (std::vector<PlexilVarRef*>::const_iterator it =
			 intf->inOut().begin(); it != intf->inOut().end(); ++it) {
		inOut->LinkEndChild(toXml(*it));
	  }
	  retval->LinkEndChild(inOut);
	}
	return retval;
  }

  // *** add support for array vars ***

  TiXmlElement* PlexilXmlParser::toXml(const PlexilVarId& var)
	throw(ParserException) {
	TiXmlElement* retval = element(std::string(var->isArray() ? DECL_ARRAY_TAG
											   : DECL_VAR_TAG));
	TiXmlElement* name = namedTextElement(NAME_TAG, var->name());
	retval->LinkEndChild(name);
	TiXmlElement* type = namedTextElement(TYPE_TAG,
										  PlexilParser::valueTypeString(var->type()));
	retval->LinkEndChild(type);

	if (var->isArray()) {
	  const PlexilArrayVarId& arrayVar = (const PlexilArrayVarId&) var;
	  // max size
	  TiXmlElement* max =
		namedNumberElement(MAXSIZE_TAG, arrayVar->maxSize());
	  retval->LinkEndChild(max);

	  // initial values
	  TiXmlElement* vals = element(INITIALVAL_TAG);
	  std::string valueTag = PlexilParser::valueTypeString(arrayVar->type())
		+ VAL_TAG;
	  const std::vector<std::string>& values =
		((PlexilArrayValue *) arrayVar->value())->values();
	  for (std::vector<std::string>::const_iterator it = values.begin(); it
			 != values.end(); it++) {
		vals->LinkEndChild(namedTextElement(valueTag, *it));
	  }
	  retval->LinkEndChild(vals);
	} else {
	  // initial value
	  retval->LinkEndChild(toXml(var->value()->getId()));
	}

	int lineno = var->lineNo();
	if (lineno != 0)
	  retval->SetAttribute(LINENO_ATTR, lineno);

	int col = var->colNo();
	if (col != 0)
	  retval->SetAttribute(COLNO_ATTR, col);

	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilExprId& expr)
	throw(ParserException) {
	TiXmlElement* result = toXml(expr.operator->());

	int lineno = expr->lineNo();
	if (lineno != 0)
	  result->SetAttribute(LINENO_ATTR, lineno);

	int col = expr->colNo();
	if (col != 0)
	  result->SetAttribute(COLNO_ATTR, col);

	return result;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilExpr* expr)
	throw(ParserException) {
	if (0 != dynamic_cast<const PlexilVarRef*> (expr))
	  return toXml((const PlexilVarRef*) expr);
	else if (0 != dynamic_cast<const PlexilOp*> (expr))
	  return toXml((const PlexilOp*) expr);
	else if (0 != dynamic_cast<const PlexilArrayElement*> (expr))
	  return toXml((const PlexilArrayElement*) expr);
	else if (0 != dynamic_cast<const PlexilLookup*> (expr))
	  return toXml((const PlexilLookup*) expr);
	else if (0 != dynamic_cast<const PlexilValue*> (expr))
	  return toXml((const PlexilValue*) expr);
	checkParserException(ALWAYS_FAIL, "Should never get here.");
	return NULL;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeBodyId& body)
	throw(ParserException) {
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
	  realBody = toXml((PlexilCommandBody*) body);
	else if (Id<PlexilLibNodeCallBody>::convertable(body))
	  realBody = toXml((PlexilLibNodeCallBody*) body);
	checkParserException(realBody != NULL, "Unknown body type.");
	retval->LinkEndChild(realBody);

	int lineno = body->lineNo();
	if (lineno != 0)
	  retval->SetAttribute(LINENO_ATTR, lineno);

	int col = body->colNo();
	if (col != 0)
	  retval->SetAttribute(COLNO_ATTR, col);

	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilVarRef* ref)
	throw(ParserException) {
	if (Id<PlexilInternalVar>::convertable(ref->getId()))
	  return PlexilXmlParser::toXml((PlexilInternalVar*) ref);
	return namedTextElement((ref->typed() ? PlexilParser::valueTypeString(
																		  ref->type()) + VAR_TAG : VAR_TAG), ref->name());
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilOp* op) throw(ParserException) {
	TiXmlElement* retval = element(op->name());
	for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin(); it
		   != op->subExprs().end(); ++it)
	  retval->LinkEndChild(toXml(*it));
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilArrayElement* op)
	throw(ParserException) {
	TiXmlElement* retval = element(ARRAYELEMENT_TAG);
	retval->LinkEndChild(namedTextElement(NAME_TAG, op->getArrayName()));
	for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin(); it
		   != op->subExprs().end(); ++it)
	  retval->LinkEndChild(toXml(*it));
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilLookup* lookup)
	throw(ParserException) {
	TiXmlElement* retval;
	if (Id<PlexilLookupNow>::convertable(lookup->getId()))
	  retval = element(LOOKUPNOW_TAG);
	else if (Id<PlexilChangeLookup>::convertable(lookup->getId()))
	  retval = toXml((PlexilChangeLookup*) lookup);
	checkParserException(retval != NULL, "Unknown lookup type.");
	toXml(lookup->state(), retval);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilValue* val)
	throw(ParserException) {
	return namedTextElement(PlexilParser::valueTypeString(val->type())
							+ VAL_TAG, val->value());
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilListBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(NODELIST_TAG);
	for (std::vector<PlexilNodeId>::const_iterator it =
		   body->children().begin(); it != body->children().end(); ++it)
	  retval->LinkEndChild(toXml(*it));
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilRequestBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(REQ_TAG);
	if (body->parent().isValid())
	  retval->LinkEndChild(toXml(body->parent()));
	toXml(body->update(), retval);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilUpdateBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(UPDATE_TAG);
	toXml(body->update(), retval);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilAssignmentBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(ASSN_TAG);
	std::vector<TiXmlElement*> dest;
	toXml(body->dest(), dest);
	for (std::vector<TiXmlElement*>::const_iterator it = dest.begin(); it
		   != dest.end(); ++it)
	  retval->LinkEndChild(*it);
	TiXmlElement* rhs = element(RHS_TAG);
	rhs->LinkEndChild(toXml(body->RHS()));
	retval->LinkEndChild(rhs);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilCommandBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(CMD_TAG);
	std::vector<TiXmlElement*> dest;
	toXml(body->dest(), dest);
	for (std::vector<TiXmlElement*>::const_iterator it = dest.begin(); it
		   != dest.end(); ++it)
	  retval->LinkEndChild(*it);
	toXml(body->state(), retval);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilLibNodeCallBody* body)
	throw(ParserException) {
	TiXmlElement* retval = element(LIBRARYNODECALL_TAG);
	retval->LinkEndChild(namedTextElement(NODEID_TAG, body->libNodeName()));

	// format variable aliases
	for (PlexilAliasMap::const_iterator it = body->aliases().begin(); it
		   != body->aliases().end(); it++) {
	  // double is key to LabelStr of formal param name
	  // expr is actual param
	  const std::pair<double, PlexilExprId>& entry = *it;
	  TiXmlElement* aliasXml = element(ALIAS_TAG);
	  aliasXml->LinkEndChild(namedTextElement(NODE_PARAMETER_TAG, LabelStr(
																		   entry.first).toString()));
	  aliasXml->LinkEndChild(toXml(entry.second));
	  retval->LinkEndChild(aliasXml);
	}

	// linked library node currently ignored
	return retval;
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilInternalVar* var)
	throw(ParserException) {
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
	retval->LinkEndChild(toXml(var->ref()));

	if (Id<PlexilTimepointVar>::convertable(var->getId())) {
	  PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
	  retval->LinkEndChild(namedTextElement(STATEVAL_TAG, tp->state()));
	  retval->LinkEndChild(namedTextElement(TIMEPOINT_TAG, tp->timepoint()));
	}
	return retval;
  }

  void PlexilXmlParser::toXml(const PlexilStateId& state, TiXmlElement* parent)
	throw(ParserException) {
	parent->LinkEndChild(namedTextElement(NAME_TAG, state->name()));
	TiXmlElement* args = element(ARGS_TAG);
	for (std::vector<PlexilExprId>::const_iterator it = state->args().begin(); it
		   != state->args().end(); ++it)
	  args->LinkEndChild(toXml(*it));
	parent->LinkEndChild(args);
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilChangeLookup* lookup)
	throw(ParserException) {
	TiXmlElement* retval = element(LOOKUPCHANGE_TAG);
	for (std::vector<PlexilExprId>::const_iterator it =
		   lookup->tolerances().begin(); it != lookup->tolerances().end(); ++it)
	  retval->LinkEndChild(toXml(*it));
	return retval;
  }

  void PlexilXmlParser::toXml(const PlexilUpdateId& update, TiXmlElement* parent)
	throw(ParserException) {
	for (std::vector<std::pair<std::string, PlexilExprId> >::const_iterator it =
		   update->pairs().begin(); it != update->pairs().end(); ++it) {
	  TiXmlElement* pair = element(PAIR_TAG);
	  pair->LinkEndChild(namedTextElement(NAME_TAG, it->first));
	  pair->LinkEndChild(toXml(it->second));
	  parent->LinkEndChild(pair);
	}
  }

  void PlexilXmlParser::toXml(const std::vector<PlexilExpr*>& src, std::vector<
							  TiXmlElement*>& dest) throw(ParserException) {
	for (std::vector<PlexilExpr*>::const_iterator it = src.begin(); it
		   != src.end(); ++it)
	  dest.push_back(toXml(*it));
  }

  TiXmlElement* PlexilXmlParser::toXml(const PlexilNodeRefId& ref)
	throw(ParserException) {
	std::string dir("");
	switch (ref->dir()) {
	case PlexilNodeRef::SELF:
	  dir = "self";
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
	  checkParserException(ALWAYS_FAIL, "Unknown direction " << ref->dir())
		;
	  break;
	}
	TiXmlElement* retval;
	if (ref->dir() == PlexilNodeRef::CHILD || ref->dir()
		== PlexilNodeRef::SIBLING)
	  retval = namedTextElement(NODEREF_TAG, ref->name());
	else
	  retval = element(NODEREF_TAG);
	retval->SetAttribute(DIR_ATTR, dir);

	int lineno = ref->lineNo();
	if (lineno != 0)
	  retval->SetAttribute(LINENO_ATTR, lineno);

	int col = ref->colNo();
	if (col != 0)
	  retval->SetAttribute(COLNO_ATTR, col);
	return retval;
  }

  TiXmlElement* PlexilXmlParser::element(const std::string& name) {
	return new TiXmlElement(name);
  }

  TiXmlElement* PlexilXmlParser::namedTextElement(const std::string& name,
												  const std::string& value) {
	TiXmlElement* retval = element(name);
	retval->LinkEndChild(new TiXmlText(value));
	return retval;
  }

  TiXmlElement* PlexilXmlParser::namedNumberElement(const std::string& name,
													const double value) {
	std::ostringstream str;
	str << value;
	return namedTextElement(name, str.str());
  }
}
