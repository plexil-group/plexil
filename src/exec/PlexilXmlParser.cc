/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
#include "resource-tags.hh"

#include <cstring>
#include <set>
#include <sstream>

using std::string;
using namespace pugi;

namespace PLEXIL 
{
  const char *PLEXIL_PLAN_TAG = "PlexilPlan";
  const char *NODE_TAG = "Node";
  const char *NODEID_TAG = "NodeId";
  const char *PRIORITY_TAG = "Priority";
  const char *INTERFACE_TAG = "Interface";
  const char *VAR_DECLS_TAG = "VariableDeclarations";
  const char *DECL_VAR_TAG = "DeclareVariable";
  const char *IN_TAG = "In";
  const char *INOUT_TAG = "InOut";
  const char *VAR_TAG = "Variable";
  const char *TYPE_TAG = "Type";
  const char *MAXSIZE_TAG = "MaxSize";
  const char *DECL_TAG = "Declare";
  const char *VAL_TAG = "Value";
  const char *ARRAY_VAL_TAG = "ArrayValue";
  const char *INITIALVAL_TAG = "InitialValue";
  const char *ASSN_TAG = "Assignment";
  const char *BODY_TAG = "NodeBody";
  const char *RHS_TAG = "RHS";
  const char *NODELIST_TAG = "NodeList";
  const char *LIBRARYNODECALL_TAG = "LibraryNodeCall";
  const char *ALIAS_TAG = "Alias";
  const char *NODE_PARAMETER_TAG = "NodeParameter";
  const char *CMD_TAG = "Command";
  const char *NAME_TAG = "Name";
  const char *INDEX_TAG = "Index";
  const char *ARGS_TAG = "Arguments";
  const char *LOOKUPNOW_TAG = "LookupNow";
  const char *LOOKUPCHANGE_TAG = "LookupOnChange";
  const char *TOLERANCE_TAG = "Tolerance";
  const char *NODEREF_TAG = "NodeRef";
  const char *STATEVAL_TAG = "NodeStateValue";
  const char *TIMEPOINT_TAG = "Timepoint";
  const char *UPDATE_TAG = "Update";
  const char *PAIR_TAG = "Pair";
  const char *COND_TAG = "Condition";

  const char *INT_TAG = "Integer";
  const char *REAL_TAG = "Real";
  const char *BOOL_TAG = "Boolean";
  const char *BLOB_TAG = "BLOB";
  const char *STRING_TAG = "String";
  const char *TIME_TAG = "Time";
  const char *ARRAY_TAG = "Array";

  const char *NODE_OUTCOME_TAG = "NodeOutcome";
  const char *NODE_FAILURE_TAG = "NodeFailure";
  const char *NODE_STATE_TAG = "NodeState";
  const char *NODE_COMMAND_HANDLE_TAG = "NodeCommandHandle";

  const char *DECL_ARRAY_TAG = "DeclareArray";
  const char *ARRAYELEMENT_TAG = "ArrayElement";

  const char *NODETYPE_ATTR = "NodeType";
  const char *DIR_ATTR = "dir";

  const char *FILENAME_ATTR = "FileName";
  const char *LINENO_ATTR = "LineNo";
  const char *COLNO_ATTR = "ColNo";

  const char *PARENT_VAL = "parent";
  const char *CHILD_VAL = "child";
  const char *SIBLING_VAL = "sibling";
  const char *SELF_VAL = "self";

  //
  // Internal error checking/reporting utilities
  //

  inline bool testTag(const char* t, const xml_node& e) {
    return e.type() == node_element && 0 == strcmp(t, e.name());
  }

  bool testTagPrefix(const char* prefix, const xml_node& e)
  {
    if (e.type() != node_element)
      return false;
    const char* valueStr = e.name();
    const size_t prefixLen = strlen(prefix);
    if (strlen(valueStr) < prefixLen)
      return false;
    return 0 == strncmp(prefix, valueStr, prefixLen);
  }

  bool testTagSuffix(const char* suffix, const xml_node& e)
  {
    if (e.type() != node_element)
      return false;
    const char* valueStr = e.name();
    const size_t valueLen = strlen(valueStr);
    const size_t suffixLen = strlen(suffix);
    if (valueLen < suffixLen)
      return false;
    return 0 == strncmp(suffix, &(valueStr[valueLen - suffixLen]), suffixLen);
  }

  inline bool hasChildElement(const xml_node& e) 
  {
    return e && e.first_child() && e.first_child().type() == node_element;
  }

  void checkTag(const char* t, const xml_node& e) {
    checkParserExceptionWithLocation(testTag(t, e),
                                     e,
                                     "XML parsing error: Expected <" << t << "> element, but got <" << e.name() << "> instead.");
  }

  void checkAttr(const char* t, const xml_node& e) {
    checkParserExceptionWithLocation(e && e.type() == node_element && e.attribute(t),
                                     e,
                                     "XML parsing error: Expected an attribute named '" << t << "' in element <" << e.name() << ">");
  }

  void checkTagPrefix(const char* t, const xml_node& e) 
  {
    checkParserExceptionWithLocation(testTagPrefix(t, e),
                                     e,
                                     "XML parsing error: Expected an element starting in '" << t << "', but instead got <" << e.name() << ">");
  }

  void checkTagSuffix(const char* t, const xml_node& e) 
  {
    checkParserExceptionWithLocation(testTagSuffix(t, e),
                                     e,
                                     "XML parsing error: Expected an element ending in '" << t << "', but instead got <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkNotEmpty(const xml_node& e) {
    checkParserExceptionWithLocation(e.first_child()
                                     && e.first_child().type() == node_pcdata
                                     && *(e.first_child().value()),
                                     e,
                                     "XML parsing error: Expected a non-empty text child of <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkHasChildElement(const xml_node& e) {
    checkParserExceptionWithLocation(hasChildElement(e),
                                     e,
                                     "XML parsing error: Expected a child element of <" << e.name() << ">");
  }

  //
  // Implementation of parser class methods
  //

  bool PlexilXmlParser::s_init = true;
  std::map<string, PlexilExprParser*> *PlexilXmlParser::s_exprParsers = NULL;
  std::map<string, PlexilBodyParser*> *PlexilXmlParser::s_bodyParsers = NULL;

  /**
   * @brief A variation of PlexilExprParser that supports parsing internal variable references.
   */
  class PlexilInternalVarParser : public PlexilExprParser
  {
  public:
    PlexilInternalVarParser() : PlexilExprParser() {};
    virtual ~PlexilInternalVarParser() {}

    static PlexilNodeRefId parseNodeReference(const xml_node& xml) 
    {
      xml_node child = xml.child(NODEID_TAG);
      //if we have an old-style node reference, we have to do a lot of work!
      if (child != NULL)
        return PlexilXmlParser::getNodeRef(child, PlexilXmlParser::getNodeParent(xml));
      else if ((child = xml.child(NODEREF_TAG)))
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
    {
      PlexilTimepointVar* retval = new PlexilTimepointVar();
      retval->setRef(parseNodeReference(xml));

      xml_node state = xml.child(STATEVAL_TAG);
      checkParserExceptionWithLocation(state,
                                       xml,
                                       "XML parsing error: Timepoint missing " << STATEVAL_TAG << " tag");
      checkNotEmpty(state);
      retval->setState(state.first_child().value());

      xml_node point = xml.child(TIMEPOINT_TAG);
      checkParserExceptionWithLocation(point,
                                       xml,
                                       "XML parsing error: Timepoint missing " << TIMEPOINT_TAG << " tag");
      checkNotEmpty(point);
      retval->setTimepoint(point.first_child().value());

      return retval->getId();
    }
  };

  class PlexilOpParser: public PlexilExprParser {
  public:
    PlexilOpParser() : PlexilExprParser() {}
    ~PlexilOpParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) 
    {
      PlexilOp* retval = new PlexilOp();
      retval->setOp(xml.name());
      for (xml_node child = xml.first_child(); 
           child;
           child = child.next_sibling())
        retval->addSubExpr(PlexilXmlParser::parseExpr(child));
      return retval->getId();
    }
  };

  class PlexilChangeLookupParser: public PlexilExprParser {
  public:
    PlexilChangeLookupParser() : PlexilExprParser() {}
    ~PlexilChangeLookupParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
      checkTag(LOOKUPCHANGE_TAG, xml);
      PlexilChangeLookup* retval = new PlexilChangeLookup();
      retval->setState(PlexilXmlParser::parseState(xml));
      for (xml_node tol = xml.child(TOLERANCE_TAG); 
           tol;
           tol = tol.next_sibling(TOLERANCE_TAG)) {
        checkHasChildElement(tol);
        retval->addTolerance(PlexilXmlParser::parseExpr(tol.first_child()));
      }
      return retval->getId();
    }
  };

  class PlexilLookupNowParser: public PlexilExprParser {
  public:
    PlexilLookupNowParser() : PlexilExprParser() {}
    ~PlexilLookupNowParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
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

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
      checkTag(ARRAYELEMENT_TAG, xml);

      // create an array element
      PlexilArrayElement* arrayElement = new PlexilArrayElement();

      // extract array name
      xml_node child = xml.first_child();
      checkTag(NAME_TAG, child);
      arrayElement->setArrayName(child.first_child().value());

      // extract index
      child = child.next_sibling();
      checkTag(INDEX_TAG, child);
      PlexilExprId indexExpr = PlexilXmlParser::parseExpr(child.first_child());
      arrayElement->addSubExpr(indexExpr);

      // return new array element
      return arrayElement->getId();
    }
  };

  class PlexilValueParser: public PlexilExprParser {
  public:
    PlexilValueParser() : PlexilExprParser() {}
    ~PlexilValueParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
      // confirm that we have a value

      checkTagSuffix(VAL_TAG, xml);

      // establish value type
      const char* tag = xml.name();
      PlexilType typ = PlexilParser::parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_TAG));
      checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                       xml,
                                       "Unrecognized value type \"" << tag << "\"");

      // check for empty value
      if (!xml.first_child() || !*(xml.first_child().value())) {
        checkParserExceptionWithLocation(typ == STRING,
                                         xml.first_child(),
                                         "Empty value is not valid for \"" << tag << "\"");
        return (new PlexilValue(typ, string()))->getId();
      }

      // return new (non-empty) value
      return (new PlexilValue(typ, xml.first_child().value()))->getId();
    }
  };

  class PlexilArrayValueParser: public PlexilExprParser {
  public:
    PlexilArrayValueParser() : PlexilExprParser() {}
    ~PlexilArrayValueParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
      // confirm that we have an array value
      checkTag(ARRAY_VAL_TAG, xml);

      // confirm that we have an element type
      checkAttr(TYPE_TAG, xml);
      const char* valueType = xml.attribute(TYPE_TAG).value();
      PlexilType valtyp = PlexilParser::parseValueType(valueType);
      checkParserExceptionWithLocation(valtyp != UNKNOWN_TYPE,
                                       xml, // *** should be the attribute object
                                       "Unknown array element Type value \"" << valueType << "\"");

      // gather elements
      std::vector<string> values;

      const xml_node& thisElement = xml.first_child();
      while (thisElement) {
        checkTagSuffix(VAL_TAG, thisElement);
        // Check type
        const char* thisElementTag = thisElement.name();
        checkParserExceptionWithLocation(0 == strncmp(thisElementTag, valueType, strlen(valueType)),
                                         thisElement,
                                         "Element type mismatch: element type \"" << thisElementTag
                                         << "\" in array value of type \"" << valueType << "\"");

        // Get array element value
        const char* thisElementValue = thisElement.first_child().value();
        if (*thisElementValue) {
          values.push_back(string(thisElementValue));
        }
        else if (valueType == STRING_TAG) {
          values.push_back(string());
        }
        else {
          // parse error - empty array element not of type string
          checkParserExceptionWithLocation(ALWAYS_FAIL,
                                           thisElement,
                                           "XML parsing error: Empty element value in array value of type '" << valueType << "'");
        }
      }

      // return new value
      return (new PlexilArrayValue(valtyp, values.size(), values))->getId();
    }
  };

  class PlexilVarRefParser: public PlexilExprParser {
  public:
    PlexilVarRefParser() : PlexilExprParser() {}
    ~PlexilVarRefParser() {}

    PlexilExprId parse(const xml_node& xml) throw(ParserException) {
      checkTagSuffix(VAR_TAG, xml);
      checkNotEmpty(xml);
      const char* tag = xml.name();
      PlexilType typ = PlexilParser::parseValueTypePrefix(tag, strlen(tag) - strlen(VAR_TAG));
      checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                       xml,
                                       "Unknown variable type \"" << tag << "\"");

      PlexilVarRef* retval = new PlexilVarRef();
      retval->setName(xml.first_child().value());
      retval->setType(typ);
      return retval->getId();
    }
  };

  class PlexilActionParser: public PlexilBodyParser {
  public:
    PlexilActionParser() : PlexilBodyParser() {}
    virtual ~PlexilActionParser() {}

    void parseDest(const xml_node& xml, PlexilActionBody* body)
    throw(ParserException) {
      for (xml_node var = xml.first_child(); 
           var; 
           var = var.next_sibling()) {
        if (testTagSuffix(VAR_TAG, var)) {
          body->addDestVar(PlexilVarRefParser().parse(var));
        }
        else if (testTag(ARRAYELEMENT_TAG, var)) {
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

    PlexilNodeBodyId parse(const xml_node& xml) throw(ParserException) {
      checkTag(ASSN_TAG, xml);
      PlexilAssignmentBody* retval = new PlexilAssignmentBody();
      parseDest(xml, retval);
      // FIXME: add check for one destination variable here
      xml_node rhs;
      for (xml_node child = xml.first_child();
           child; 
           child = child.next_sibling()) {
        if (testTagSuffix(RHS_TAG, child)) {
          // *** N.B. Used to try to get expression type info here,
          // but that info is ambiguous and not used at present.
          // So ignore it.
          rhs = child;
          break;
        }
      }

      checkParserExceptionWithLocation(rhs,
                                       xml,
                                       "XML parsing error: Missing RHS (return value) tags for " << xml.name());
      checkParserExceptionWithLocation(rhs.first_child(),
                                       rhs,
                                       "XML parsing error: Empty RHS (return value) tags for " << xml.name());
      retval->setRHS(PlexilXmlParser::parseExpr(rhs.first_child()));
      return retval->getId();
    }
  };

  class PlexilNodeListParser: public PlexilBodyParser {
  public:
    PlexilNodeListParser() : PlexilBodyParser() {}
    ~PlexilNodeListParser() {}

    PlexilNodeBodyId parse(const xml_node& xml) throw(ParserException) {
      checkTag(NODELIST_TAG, xml);
      PlexilListBody* retval = new PlexilListBody();
      std::set<string> childIds;
      for (xml_node child = xml.child(NODE_TAG); 
           child;
           child = child.next_sibling(NODE_TAG)) {
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

    PlexilNodeBodyId parse(const xml_node& xml) throw(ParserException) {
      checkTag(LIBRARYNODECALL_TAG, xml);

      // get node id
      xml_node nodeIdXml = xml.child(NODEID_TAG);
      checkParserExceptionWithLocation(nodeIdXml,
                                       xml,
                                       "XML parsing error: Missing <NodeId> element in library call.");
      const char* nodeId = nodeIdXml.first_child().value();
      checkParserExceptionWithLocation(*nodeId,
                                       nodeIdXml,
                                       "XML parsing error: Empty <NodeId> element in library call.");

      // create lib node call node body
      PlexilLibNodeCallBody* body = new PlexilLibNodeCallBody(nodeId);

      // collect the variable alias information
      // FIXME (?): ignores junk before/between/around aliases
      for (xml_node child = xml.child(ALIAS_TAG); 
           child;
           child = child.next_sibling(ALIAS_TAG)) {

        // get library node parameter
        const xml_node& libParamXml = child.child(NODE_PARAMETER_TAG);
        checkParserExceptionWithLocation(libParamXml,
                                         child,
                                         "XML parsing library error: Missing <NodeParameter> element in library call.");
        const char* libParam = libParamXml.first_child().value();
        checkParserExceptionWithLocation(*libParam,
                                         libParamXml,
                                         "XML parsing library error: Empty <NodeParameter> element in library call.");

        // get node parameter value
        PlexilExprId value = PlexilXmlParser::parseExpr(libParamXml.next_sibling());

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

    PlexilNodeBodyId parse(const xml_node& xml) throw(ParserException) {
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

    PlexilUpdateId parsePairs(const xml_node& xml) throw(ParserException) {
      PlexilUpdateId retval = (new PlexilUpdate())->getId();
      for (xml_node pair = xml.child(PAIR_TAG);
           pair;
           pair = pair.next_sibling(PAIR_TAG)) {
        xml_node nameElt = pair.first_child();
        checkTag(NAME_TAG, nameElt);
        const char* name = nameElt.first_child().value();
        xml_node value = nameElt.next_sibling();
        checkParserExceptionWithLocation(value,
                                         pair,
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

    PlexilNodeBodyId parse(const xml_node& xml) throw(ParserException) {
      checkTag(UPDATE_TAG, xml);
      PlexilUpdateBody* retval = new PlexilUpdateBody();
      if (xml.child(PAIR_TAG))
        retval->setUpdate(parsePairs(xml));
      return retval->getId();
    }
  };

  void PlexilXmlParser::registerParsers() 
  {
    if (s_init) {
      s_bodyParsers = new std::map<string, PlexilBodyParser*>();
      s_bodyParsers->insert(std::make_pair(ASSN_TAG, new PlexilAssignmentParser()));
      s_bodyParsers->insert(std::make_pair(NODELIST_TAG,
                                           new PlexilNodeListParser()));
      s_bodyParsers->insert(std::make_pair(LIBRARYNODECALL_TAG,
                                           new PlexilLibraryNodeCallParser()));
      s_bodyParsers->insert(std::make_pair(CMD_TAG, new PlexilCommandParser()));
      s_bodyParsers->insert(std::make_pair(UPDATE_TAG, new PlexilUpdateParser()));

      s_exprParsers = new std::map<string, PlexilExprParser*>();
      PlexilExprParser* varRef = new PlexilVarRefParser();
      s_exprParsers->insert(std::make_pair(VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(INT_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(REAL_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(ARRAY_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(STRING_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(BOOL_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(TIME_TAG) + VAR_TAG, varRef));
      s_exprParsers->insert(std::make_pair(string(BLOB_TAG) + VAR_TAG, varRef));

      s_exprParsers->insert(std::make_pair(string(NODE_OUTCOME_TAG) + VAR_TAG,
                                           new PlexilOutcomeVarParser()));
      s_exprParsers->insert(std::make_pair(string(NODE_FAILURE_TAG) + VAR_TAG,
                                           new PlexilFailureVarParser()));
      s_exprParsers->insert(std::make_pair(string(NODE_STATE_TAG) + VAR_TAG,
                                           new PlexilStateVarParser()));
      s_exprParsers->insert(std::make_pair(string(NODE_COMMAND_HANDLE_TAG) + VAR_TAG,
                                           new PlexilCommandHandleVarParser()));
      s_exprParsers->insert(std::make_pair(string("NodeTimepoint") + VAL_TAG,
                                           new PlexilTimepointVarParser()));

      PlexilExprParser* val = new PlexilValueParser();
      s_exprParsers->insert(std::make_pair(string(INT_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(REAL_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(STRING_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(BOOL_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(TIME_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(BLOB_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(NODE_OUTCOME_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(NODE_FAILURE_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(NODE_STATE_TAG) + VAL_TAG, val));
      s_exprParsers->insert(std::make_pair(string(NODE_COMMAND_HANDLE_TAG) + VAL_TAG, val));
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
      s_exprParsers->insert(std::make_pair("MAX", op));
      s_exprParsers->insert(std::make_pair("MIN", op));
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
  PlexilNodeId PlexilXmlParser::findLibraryNode(const string& name,
                                                const std::vector<string>& path)
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
  PlexilNodeId PlexilXmlParser::findPlan(const string& name,
                                         const string& fileName,
                                         const std::vector<string>& path)
  {
    debugMsg("PlexilXmlParser:findPlan", "(\"" << name << "\", \"" << fileName << "\")");
    PlexilNodeId result;
    std::vector<string>::const_iterator it = path.begin();
    string fileNameWithSuffix = fileName;
    // add suffix if lacking
    if (fileName.length() <= 4 ||
        fileName.compare(fileName.length() - 4, 4, ".plx") != 0)
      fileNameWithSuffix += ".plx";

    // Find the first occurrence of the library in this path
    while (result.isNoId() && it != path.end()) {
      string candidateFile = *it + "/" + fileNameWithSuffix;
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
      ++it;
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
  PlexilNodeId PlexilXmlParser::loadPlanNamed(const string& name, 
                                              const string& filename)
    throw(ParserException)
  {
    debugMsg("PlexilXmlParser:loadPlanNamed", "(\"" << name << "\", \"" << filename << "\")");
    xml_document doc;
    xml_parse_result parseResult = doc.load_file(filename.c_str(), PUGI_PARSE_OPTIONS());
    if (parseResult.status != status_ok) {
      checkParserException(parseResult.status != status_file_not_found,
                           "Error reading XML file " << filename
                           << ": " << parseResult.description());
      debugMsg("PlexilXmlParser:loadPlanNamed", 
               " unable to parse file " << filename
               << " (offset " << parseResult.offset << "): "
               << parseResult.description());
      return PlexilNodeId::noId();
    }
    PlexilNodeId result = parse(doc.document_element());
    // Check that node has the desired node ID
    checkParserException(0 == name.compare(result->nodeId()),
                         "Error: File " << filename
                         << " contains node ID \"" << result->nodeId()
                         << "\", not \"" << name << "\"");
    debugMsg("PlexilXmlParser:loadPlanNamed",
             " successfully loaded node " << name << " from " << filename);
    return result;
  }

  PlexilNodeId PlexilXmlParser::parse(const string& str, bool isFile)
    throw(ParserException) 
  {
    if (!isFile)
      return parse(str.c_str());

    registerParsers();
    xml_document doc;
    xml_parse_result parseResult = doc.load_file(str.c_str(), PUGI_PARSE_OPTIONS());
    checkParserException(parseResult.status != status_ok,
                         "Error reading XML file " << str << ": " << parseResult.description());
    xml_node root = doc.document_element();
    checkParserException(root != NULL, "XML parsing error: No XML document in file " << str);
    PlexilNodeId result = parse(root);
    if (result->fileName().empty())
      result->setFileName(str);
    return result;
  }

  PlexilNodeId PlexilXmlParser::parse(const char* text)
  throw(ParserException) 
  {
    // First parse the XML itself
    xml_document doc;
    xml_parse_result parseResult = doc.load(text, PUGI_PARSE_OPTIONS());
    checkParserException(parseResult.status != status_ok,
                         "(character " << parseResult.offset
                         << ") XML parsing error: " << parseResult.description());

    xml_node root = doc.document_element();
    PlexilNodeId result = parse(root);
    return result;
  }

  PlexilNodeId PlexilXmlParser::parse(xml_node xml)
  throw(ParserException) 
  {
    registerParsers();
    // strip away PlexilPlan wrapper, if any
    if (testTag(PLEXIL_PLAN_TAG, xml)) {
      // TODO: parse global declarations
      xml_node node = xml.child(NODE_TAG);
      checkParserExceptionWithLocation(node,
                                       xml,
                                       "XML parsing error: No root node found");
      xml = node;
    }
    PlexilNodeId result = parseNode(xml);
    // FIXME: Add post-parse checks (e.g. node and variable accessibility) here
    return result;
  }

  PlexilExprId PlexilXmlParser::parseExpr(const xml_node& xml)
  throw(ParserException) {
    std::map<string, PlexilExprParser*>::iterator it =
      s_exprParsers->find(std::string(xml.name()));
    checkParserExceptionWithLocation(it != s_exprParsers->end(),
                                     xml,
                                     "XML parsing error: No parser for expression '" << xml.name() << "'");
    return it->second->parse(xml);
  }

  PlexilNodeId PlexilXmlParser::parseNode(const xml_node& xml)
  throw(ParserException) 
  {
    checkTag(NODE_TAG, xml);
    PlexilNodeId retval = (new PlexilNode())->getId();

    // nodeid required

    xml_node nodeIdXml = xml.child(NODEID_TAG);
    checkParserExceptionWithLocation(nodeIdXml,
                                     xml,
                                     "XML parsing error: Missing <NodeId> element.");
    const char* nodeId = nodeIdXml.first_child().value();
    checkParserExceptionWithLocation(*nodeId,
                                     nodeIdXml,
                                     "XML parsing error: Empty <NodeId> element.");
    retval->setNodeId(std::string(nodeId));

    // node type required

    checkAttr(NODETYPE_ATTR, xml);
    PlexilNodeType ntype = parseNodeType(xml.attribute(NODETYPE_ATTR).value());
    checkParserExceptionWithLocation(ntype != NodeType_error,
                                     xml, // FIXME: better location would help
                                     "XML parsing error: " << NODETYPE_ATTR
                                     << " attribute value \"" << xml.attribute(NODETYPE_ATTR).value()
                                     << "\" is not a valid node type");
    retval->setNodeType(ntype);

    // file name, line, col optional
    const char* fname = xml.attribute(FILENAME_ATTR).value();
    if (*fname)
      retval->setFileName(fname);
    int line = xml.attribute(LINENO_ATTR).as_int();
    if (line != 0)
      retval->setLineNo(line);
    int col = xml.attribute(COLNO_ATTR).as_int();
    if (col != 0)
      retval->setColNo(col);

    // priority optional

    xml_node priorityXml = xml.child(PRIORITY_TAG);
    if (priorityXml) {
      const char* priority = priorityXml.first_child().value();
      if (*priority) {
        std::istringstream str(priority);
        double value;
        str >> value;
        retval->setPriority(value);
      }
    }

    // interface optional

    xml_node interfaceXml = xml.child(INTERFACE_TAG);
    if (interfaceXml)
      retval->setInterface(parseInterface(interfaceXml));

    // variable declarations optional

    xml_node declarationsXml = xml.child(VAR_DECLS_TAG);
    if (declarationsXml)
      parseDeclarations(declarationsXml, retval);

    // conditions optional

    for (xml_node conditionsXml = xml.first_child(); 
         conditionsXml;
         conditionsXml = conditionsXml.next_sibling()) {
      if (!testTagSuffix(COND_TAG, conditionsXml))
        continue;

      // Check here so we don't blow up when we load the plan
      checkParserExceptionWithLocation(isValidConditionName(conditionsXml.name()),
                                       conditionsXml,
                                       "XML parsing error: " << conditionsXml.name()
                                       << " is not a valid condition name");
      retval->addCondition(conditionsXml.name(), 
                           parseExpr(conditionsXml.first_child()));
    }

    // node body
    xml_node bodyXml = xml.child(BODY_TAG);
    if (!bodyXml) {
      checkParserExceptionWithLocation(retval->nodeType() == NodeType_Empty,
                                       xml,
                                       "XML parsing error: " << retval->nodeTypeString()
                                       << " node \"" << retval->nodeId() <<
                                       "\" lacks a <NodeBody> element.");
    }
    else {
      xml_node realBodyXml = bodyXml.first_child();
      if (realBodyXml) {
        PlexilNodeBodyId bodyStruct = parseBody(realBodyXml);

        // Check that body is of correct type
        switch (retval->nodeType()) {
        case NodeType_NodeList:
          // check for PlexilListBody
          checkParserExceptionWithLocation(NULL != (PlexilListBody*) bodyStruct,
                                           realBodyXml,
                                           "XML parsing error: Body of " << retval->nodeTypeString()
                                           << " node \"" << retval->nodeId()
                                           << "\" contains a " << realBodyXml.name() << " element.")
            break;

        case NodeType_Command:
          // check for PlexilCommandBody
          checkParserExceptionWithLocation(NULL != (PlexilCommandBody*) bodyStruct,
                                           realBodyXml,
                                           "XML parsing error: Body of " << retval->nodeTypeString()
                                           << " node \"" << retval->nodeId()
                                           << "\" contains a " << realBodyXml.name() << " element.")
            break;

        case NodeType_Assignment:
          // check for PlexilAssignmentBody
          checkParserExceptionWithLocation(NULL != (PlexilAssignmentBody*) bodyStruct,
                                           realBodyXml,
                                           "XML parsing error: Body of " << retval->nodeTypeString()
                                           << " node \"" << retval->nodeId()
                                           << "\" contains a " << realBodyXml.name() << " element.")
            break;

        case NodeType_Update:
          // check for PlexilUpdateBody
          checkParserExceptionWithLocation(NULL != (PlexilUpdateBody*) bodyStruct,
                                           realBodyXml,
                                           "XML parsing error: Body of " << retval->nodeTypeString()
                                           << " node \"" << retval->nodeId()
                                           << "\" contains a " << realBodyXml.name() << " element.")
            break;

        case NodeType_LibraryNodeCall:
          // check for PlexilLibNodeCallBody
          checkParserExceptionWithLocation(NULL != (PlexilLibNodeCallBody*) bodyStruct,
                                           realBodyXml,
                                           "XML parsing error: Body of " << retval->nodeTypeString()
                                           << " node \"" << retval->nodeId()
                                           << "\" contains a " << realBodyXml.name() << " element.")
            break;

        case NodeType_Empty:
          checkParserExceptionWithLocation(ALWAYS_FAIL,
                                           bodyXml,
                                           "XML parsing error: Empty node \"" << retval->nodeId()
                                           << "\" may not contain a " << bodyXml.name() << " element.")
            break;

        default:
          checkParserExceptionWithLocation(ALWAYS_FAIL,
                                           xml,
                                           "XML parser internal error: Invalid node type while parsing node body");
          break;
        }
        retval->setBody(bodyStruct);
      }
    }

    return retval;
  }
  
  bool PlexilXmlParser::isValidConditionName(const string& name)
  {
    return name == START_CONDITION_TAG()
      || name == REPEAT_CONDITION_TAG()
      || name == PRE_CONDITION_TAG()
      || name == POST_CONDITION_TAG()
      || name == INVARIANT_CONDITION_TAG()
      || name == END_CONDITION_TAG()
      || name == EXIT_CONDITION_TAG()
      || name == SKIP_CONDITION_TAG();
  }

  PlexilInterfaceId PlexilXmlParser::parseDeprecatedInterface(const xml_node& intf)
  throw(ParserException) {
    checkTag(INTERFACE_TAG, intf);
    PlexilInterfaceId retval = (new PlexilInterface())->getId();
    xml_node in = intf.child(IN_TAG);
    PlexilVarRefParser p;
    if (in)
      for (xml_node var = in.first_child(); 
           var;
           var = var.next_sibling())
        retval->addIn(p.parse(var));
    xml_node inOut = intf.child(INOUT_TAG);
    if (inOut)
      for (xml_node var = inOut.first_child(); 
           var;
           var = var.next_sibling())
        retval->addInOut(p.parse(var));
    return retval;
  }

  PlexilInterfaceId PlexilXmlParser::parseInterface(const xml_node& intf)
  throw(ParserException) {
    PlexilInterfaceId retval = (new PlexilInterface())->getId();
    checkTag(INTERFACE_TAG, intf);
    parseInOrInOut(intf.child(IN_TAG), retval, false);
    parseInOrInOut(intf.child(INOUT_TAG), retval, true);
    return retval;
  }

  void PlexilXmlParser::parseInOrInOut(const xml_node& inOrInOut,
                                       PlexilInterfaceId& interface, 
                                       bool isInOut)
    throw(ParserException) 
  {
    // if this is an empty In or InOut section, just return
    if (!inOrInOut || !inOrInOut.first_child())
      return;

    for (xml_node var = inOrInOut.first_child(); 
         var; 
         var = var.next_sibling()) {

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
      }
    }
  }

  void PlexilXmlParser::parseDeclarations(const xml_node& decls,
                                          PlexilNodeId &node)
    throw(ParserException) 
  {
    checkTag(VAR_DECLS_TAG, decls);
    for (xml_node decl = decls.first_child();
         decl;
         decl = decl.next_sibling())
      node->addVariable(parseDeclaration(decl)->getId());
  }

  PlexilVar* PlexilXmlParser::parseDeclaration(const xml_node& decl)
    throw(ParserException) {
    // if array declaration

    if (testTag(DECL_ARRAY_TAG, decl))
      return parseArrayDeclaration(decl);

    // if variable declaration

    if (testTag(DECL_VAR_TAG, decl))
      return parseAtomicOrStringDeclaration(decl);

    // else it's an error
    checkParserExceptionWithLocation(ALWAYS_FAIL,
                                     decl,
                                     "Expected a " << DECL_VAR_TAG << " or " << DECL_ARRAY_TAG
                                     << ", but found \"" << decl.name() << "\"");
    return NULL;
  }

  // parse an array declaration

  PlexilVar* PlexilXmlParser::parseArrayDeclaration(const xml_node& decl)
    throw(ParserException) {
    checkTag(DECL_ARRAY_TAG, decl);

    // extract array name

    xml_node child = decl.first_child();
    checkTag(NAME_TAG, child);
    const char* name = child.first_child().value();

    // extract array type

    child = child.next_sibling();
    checkTag(TYPE_TAG, child);
    const char* typnam = child.first_child().value();
    PlexilType typ = parseValueType(typnam);
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     child.first_child(),
                                     "Unknown type name \"" << typnam << "\"");

    // extract array max size

    child = child.next_sibling();
    checkTag(MAXSIZE_TAG, child);
    std::istringstream maxSizeStr(child.first_child().value());
    unsigned int maxSize;
    maxSizeStr >> maxSize;

    debugMsg("PlexilXmlParser:parseArrayDeclaration",
             " for array " << name << ", element type " << typnam << ", size " << maxSize);

    // if present, extract initial values
    // FIXME: share code with PlexilArrayValueParser

    std::vector<string> initVals;
    if ((child = child.next_sibling())) {
      checkTag(INITIALVAL_TAG, child);
      child = child.first_child();
      do {
        checkTagSuffix(VAL_TAG, child);
        checkParserExceptionWithLocation(testTagPrefix(typnam, child),
                                         child,
                                         "XML parsing error: Initial value of " << typnam << " array variable \'" <<
                                         name << "\' of incorrect type \'" << child.value() << "\'");
        string initVal(child.first_child().value());
        initVals.push_back(initVal);
        debugMsg("PlexilXmlParser:parseArrayDeclaration",
                 " element value \"" << initVal << "\"");
        checkParserExceptionWithLocation(initVals.size() <= maxSize,
                                         child.first_child(),
                                         "XML parsing error: Number of initial values of " << typnam <<
                                         " array variable \'" << name <<
                                         "\' exceeds maximum of " << maxSize);
      } while ((child = child.next_sibling()));
    }

    // add variable to returned variable set

    PlexilVar* result = new PlexilArrayVar(name, typ, maxSize, initVals);
    debugMsg("PlexilXmlParser:parseArrayDeclaration", " succeeded");
    return result;
  }

  // parse an atomic or string declaration

  PlexilVar* PlexilXmlParser::parseAtomicOrStringDeclaration(const xml_node& decl) 
    throw(ParserException) 
  {
    checkTag(DECL_VAR_TAG, decl);

    // extract name
    xml_node child = decl.first_child();
    checkTag(NAME_TAG, child);
    const string& name = child.first_child().value();

    // extract type
    child = child.next_sibling();
    checkTag(TYPE_TAG, child);
    const char* typnam = child.first_child().value();
    PlexilType typ = parseValueType(typnam);
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     child.first_child(),
                                     "Unknown type name \"" << typnam << "\"");

    // if present, create variable with initial value
    if ((child = child.next_sibling())) {
      checkTag(INITIALVAL_TAG, child);
      child = child.first_child();
      checkTagSuffix(VAL_TAG, child);
      checkParserExceptionWithLocation(testTagPrefix(typnam, child),
                                       child,
                                       "XML parsing error: Initial value of " << typnam << " variable \'" <<
                                       name << "\' of incorrect type \'" << child.name() << "\'");
      checkParserExceptionWithLocation(typ == STRING || *(child.first_child().value()),
                                       child.first_child(),
                                       "XML parsing error: Initial value of " << typnam << " variable \'" <<
                                       name << "\' may not be empty");
      return new PlexilVar(name, typ, child.first_child().value());
    }

    // otherwise create variable with the value unknown
    return new PlexilVar(name, typ);
  }

  PlexilNodeBodyId PlexilXmlParser::parseBody(const xml_node& body)
  throw(ParserException) {
    const char* name = body.name();
    std::map<string, PlexilBodyParser*>::iterator it = s_bodyParsers->find(string(name));
    checkParserExceptionWithLocation(it != s_bodyParsers->end(),
                                     body,
                                     "XML parsing error: No parser for body type \"" << name << "\"");
    return it->second->parse(body);
  }

  // N.B. Tolerance comes between Name and Arguments in a LookupOnChange.
  // N.B. Also used by Command parser, where position of Name is flexible (may be 1st - 3rd element!).
  PlexilStateId PlexilXmlParser::parseState(const xml_node& xml)
  throw(ParserException) 
  {
    PlexilStateId retval = (new PlexilState())->getId();
    xml_node nameElt = xml.child(NAME_TAG);
    checkParserExceptionWithLocation(nameElt,
                                     xml,
                                     "Missing " << NAME_TAG << " element in \"" << xml.name() << "\"");
    checkParserExceptionWithLocation(nameElt.first_child(),
                                     nameElt,
                                     "Empty " << NAME_TAG << " element in \"" << xml.name() << "\"");
    retval->setNameExpr(parseExpr(nameElt.first_child()));
    
    xml_node arguments = xml.child(ARGS_TAG);
    if (arguments) {
      for (xml_node child = arguments.first_child();
           child;
           child = child.next_sibling()) {
        retval->addArg(PlexilXmlParser::parseExpr(child));
      }
    }

    return retval;
  }

  std::vector<PlexilResourceId> 
  PlexilXmlParser::parseResource(const xml_node& xml) 
    throw(ParserException) 
  {
    // Create a vector of PlexilResourceId
    std::vector<PlexilResourceId> rId_vec;

    // ResourceList tag is supposed to be first element in Command
    xml_node child = xml.first_child();
    if (testTag(RESOURCE_LIST_TAG, child)) {
      // Loop through each resource in the list
      for (xml_node resourceElt = child.first_child(); 
           resourceElt;
           resourceElt = resourceElt.next_sibling()) {

        checkTag(RESOURCE_TAG, resourceElt);

        // check that the resource has a name and a priority
        checkParserExceptionWithLocation(resourceElt.child(RESOURCE_NAME_TAG),
                                         resourceElt,
                                         "XML parsing error: No " << RESOURCE_NAME_TAG << " element for resource");
        checkParserExceptionWithLocation(resourceElt.child(RESOURCE_PRIORITY_TAG),
                                         resourceElt,
                                         "XML parsing error: No " << RESOURCE_PRIORITY_TAG << " element for resource");

        // Create a new PlexilResourceId.
        PlexilResourceId prId = (new PlexilResource())->getId();
        // loop through each resource element
        for (xml_node child3 = resourceElt.first_child(); 
             child3; 
             child3 = child3.next_sibling()) {
          // add each resource element just like addArg to PLexilResourceId. Use
          // tag3 and expresssion the in <name, expr> pair
          prId->addResource(child3.name(),
                            PlexilXmlParser::parseExpr(child3.first_child()));
        }

        // push the PlexilResourceId into a vector to be returned and
        // used in the PlexilCommandBody.
        rId_vec.push_back(prId);
      }
    }

    // return the vector of PlexilResourceId
    return rId_vec;
  }

  PlexilNodeRefId PlexilXmlParser::parseNodeRef(const xml_node& ref)
  throw(ParserException) {
    checkTag(NODEREF_TAG, ref);
    checkAttr(DIR_ATTR, ref);
    PlexilNodeRef::Direction dir = PlexilNodeRef::NO_DIR;
    const char* dirValue = ref.attribute(DIR_ATTR).value();
    if (0 == strcmp(dirValue, CHILD_VAL))
      dir = PlexilNodeRef::CHILD;
    else if (0 == strcmp(dirValue, PARENT_VAL))
      dir = PlexilNodeRef::PARENT;
    else if (0 == strcmp(dirValue, SIBLING_VAL))
      dir = PlexilNodeRef::SIBLING;
    else if (0 == strcmp(dirValue, SELF_VAL))
      dir = PlexilNodeRef::SELF;
    else {
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       ref,
                                       "XML parsing error: Invalid value for " << DIR_ATTR << " attibute \""
                                       << dirValue << "\"");
    }
    if (dir == PlexilNodeRef::PARENT
        && dir == PlexilNodeRef::SELF)
      return (new PlexilNodeRef(dir))->getId();

    checkNotEmpty(ref);
    return (new PlexilNodeRef(dir, ref.first_child().value()))->getId();
  }

  void PlexilXmlParser::getNameOrValue(const xml_node& xml,
                                       string& name, 
                                       string& value) {
    if (!xml)
      return;
    if (testTagSuffix(VAR_TAG, xml))
      name = xml.first_child().value();
    else if (testTagSuffix(VAL_TAG, xml)) {
      if (xml.first_child())
        value = xml.first_child().value();
    }
  }

  // returns the first XML element up the tree with a tag of NODE_TAG
  xml_node PlexilXmlParser::getNodeParent(const xml_node& node) 
  {
    xml_node parent = node.parent();
    while (parent) {
      if (testTag(NODE_TAG, parent))
        return parent; // success
      parent = parent.parent();
    }
    // no such parent found, return empty xml_node
    return parent;
  }

  PlexilNodeRefId PlexilXmlParser::getNodeRef(const xml_node& ref,
                                              const xml_node& node)
    throw(ParserException) 
  {
    checkTag(NODEID_TAG, ref);
    const char* name = ref.first_child().value();

    debugMsg("PlexilXmlParser:getNodeRef", " for \"" << name << "\"");

    PlexilNodeRefId result = getNodeRefInternal(name, node, node, ref);
    checkParserExceptionWithLocation(result.isValid(),
                                     ref.first_child(),
                                     "Node '" << node.child(NODEID_TAG).first_child().value()
                                     << "' is trying to access node '" << name
                                     << "' which is out of scope or does not exist");

    return result;
  }

  PlexilNodeRefId PlexilXmlParser::getNodeRefInternal(const char* name,
                                                      const xml_node& node,
                                                      const xml_node& referringNode,
                                                      const pugi::xml_node& ref)
    throw(ParserException) 
  {
    checkTag(NODE_TAG, node);

    xml_node foundSelf, foundChild;
    xml_node selfId = node.child(NODEID_TAG);
    if (selfId
        && selfId.first_child()
        && 0 == strcmp(name, selfId.first_child().value())) {
      debugMsg("PlexilXmlParser:getNodeRef", " name matches self");
      foundSelf = node;
    }

    // Check this node's children, if any
    xml_node checkChild = node.child(BODY_TAG);
    if (checkChild && (checkChild = checkChild.child(NODELIST_TAG))) {
      for (checkChild = checkChild.child(NODE_TAG);
           checkChild;
           checkChild = checkChild.next_sibling(NODE_TAG)) {
        if (checkChild != referringNode) {
          xml_node childId = checkChild.child(NODEID_TAG);
          if (childId
              && childId.first_child()
              && 0 == strcmp(name, childId.first_child().value())) {
            // FIXME: has the duplicate naming error been discovered by now?
            checkParserExceptionWithLocation(!foundChild,
                                             checkChild,
                                             "Sibling nodes have same node id '" << name << "'");
            debugMsg("PlexilXmlParser:getNodeRef", " name matches child");
            foundChild = checkChild;
          }
        }
      }
    }

    // FIXME: has the duplicate naming error been discovered by now?
    checkParserExceptionWithLocation(!(foundSelf && foundChild),
                                     ref,
                                     "Ambiguous node reference: Node and its parent have the same node ID '"
                                     << name << "'");

    if (foundSelf)
      return (new PlexilNodeRef(PlexilNodeRef::SELF, name))->getId();
    else if (foundChild)
      return (new PlexilNodeRef(PlexilNodeRef::CHILD, name))->getId();

    // Recurse
    xml_node parent = getNodeParent(node);
    if (parent) {
      debugMsg("PlexilXmlParser:getNodeRef", " checking parent node");
      PlexilNodeRefId result = getNodeRefInternal(name, parent, referringNode, ref);
      if (result.isId())
        result->incrementGeneration();
      return result;
    }

    return PlexilNodeRefId::noId();
  }

  //
  // PlexilNode -> XML
  //

  // Helper functions
  xml_node appendElement(const char* name, xml_node& parent) 
  {
    xml_node retval = parent.append_child();
    retval.set_name(name);
    return retval;
  }

  xml_node appendNamedTextElement(const char* name,
                                  const char* value, 
                                  xml_node& parent) 
  {
    xml_node retval = parent.append_child();
    retval.set_name(name);
    xml_node text = retval.append_child(node_pcdata);
    text.set_value(value);
    return retval;
  }

  xml_node appendNamedNumberElement(const char* name,
                                    const double value,
                                    xml_node& parent)
  {
    std::ostringstream str;
    str << value;
    return appendNamedTextElement(name, str.str().c_str(), parent);
  }

  template <typename T>
  void addSourceLocators(xml_node& xml, T obj)
  {
    int lineno = obj->lineNo();
    if (lineno != 0)
      xml.append_attribute(LINENO_ATTR).set_value(lineno);

    int col = obj->colNo();
    if (col != 0)
      xml.append_attribute(COLNO_ATTR).set_value(col);
  }

  // Main entry point
  xml_document* PlexilXmlParser::toXml(const PlexilNodeId& node)
    throw(ParserException) 
  {
    xml_document* result = new xml_document();
    toXml(node, *result);
    return result;
  }

  void PlexilXmlParser::toXml(const PlexilNodeId& node, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(NODE_TAG, parent);
    retval.append_attribute(NODETYPE_ATTR).set_value(node->nodeTypeString().c_str());
    const string & filename = node->fileName();
    if (!filename.empty())
      retval.append_attribute(FILENAME_ATTR).set_value(filename.c_str());

    appendNamedTextElement(NODEID_TAG, node->nodeId().c_str(), retval);
    appendNamedNumberElement(PRIORITY_TAG, node->priority(), retval);

    addSourceLocators(retval, node);

    if (node->interface().isId())
      toXml(node->interface(), retval);

    if (!node->declarations().empty()) {
      xml_node declarations = appendElement(VAR_DECLS_TAG, retval);
      for (std::vector<PlexilVarId>::const_iterator it = node->declarations().begin(); 
           it != node->declarations().end(); 
           ++it)
        toXml(*it, declarations);
    }

    for (std::map<string, PlexilExprId>::const_iterator it = node->conditions().begin(); 
         it != node->conditions().end();
         ++it) {
      xml_node cond = appendElement(it->first.c_str(), retval);
      toXml(it->second, cond);
    }

    if (node->body().isValid())
      toXml(node->body(), retval);
  }

  void PlexilXmlParser::toXml(const PlexilInterfaceId& intf, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(INTERFACE_TAG, parent);
    if (!intf->in().empty()) {
      xml_node in = appendElement(IN_TAG, retval);
      for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
           it != intf->in().end();
           ++it)
        toXml(*it, in);
    }
    if (!intf->inOut().empty()) {
      xml_node inOut = appendElement(INOUT_TAG, retval);
      for (std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin(); 
           it != intf->inOut().end();
           ++it) 
        toXml(*it, inOut);
    }
  }

  void PlexilXmlParser::toXml(const PlexilVarId& var, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement((var->isArray() ? DECL_ARRAY_TAG : DECL_VAR_TAG),
                                    parent);
    appendNamedTextElement(NAME_TAG, var->name().c_str(), retval);
    appendNamedTextElement(TYPE_TAG, valueTypeString(var->type()).c_str(), retval);

    if (var->isArray()) {
      const PlexilArrayVarId& arrayVar = (const PlexilArrayVarId&) var;
      // max size
      appendNamedNumberElement(MAXSIZE_TAG, arrayVar->maxSize(), retval);

      // initial values
      xml_node vals = appendElement(INITIALVAL_TAG, retval);
      string valueTag = valueTypeString(arrayVar->type())
        + VAL_TAG;
      const std::vector<string>& values =
        ((PlexilArrayValue *) arrayVar->value())->values();
      for (std::vector<string>::const_iterator it = values.begin();
           it != values.end();
           ++it) 
        appendNamedTextElement(valueTag.c_str(), it->c_str(), vals);
    }
    else if (var->value() != NULL) {
      // initial value
      toXml(var->value()->getId(), retval);
    }

    addSourceLocators(retval, var);
  }

  void PlexilXmlParser::toXml(const PlexilExprId& exprId, xml_node& parent)
    throw(ParserException)
  {
    toXml(exprId.operator->(), parent);
  }

  void PlexilXmlParser::toXml(const PlexilExpr* expr, xml_node& parent)
    throw(ParserException)
  {

    if (0 != dynamic_cast<const PlexilVarRef*> (expr))
      toXml((const PlexilVarRef*) expr, parent);
    else if (0 != dynamic_cast<const PlexilOp*> (expr))
      toXml((const PlexilOp*) expr, parent);
    else if (0 != dynamic_cast<const PlexilArrayElement*> (expr))
      toXml((const PlexilArrayElement*) expr, parent);
    else if (0 != dynamic_cast<const PlexilLookup*> (expr))
      toXml((const PlexilLookup*) expr, parent);
    else if (0 != dynamic_cast<const PlexilValue*> (expr))
      toXml((const PlexilValue*) expr, parent);
    else
      checkParserException(ALWAYS_FAIL, "Should never get here.");

    // *** FIXME: add source locators to called fns above ***
  }

  void PlexilXmlParser::toXml(const PlexilNodeBodyId& body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(BODY_TAG, parent);
    xml_node realBody;
    if (Id<PlexilListBody>::convertable(body))
      toXml((PlexilListBody*) body, retval);
    else if (Id<PlexilUpdateBody>::convertable(body))
      toXml((PlexilUpdateBody*) body, retval);
    else if (Id<PlexilAssignmentBody>::convertable(body))
      toXml((PlexilAssignmentBody*) body, retval);
    else if (Id<PlexilCommandBody>::convertable(body))
      toXml((PlexilCommandBody*) body, retval);
    else if (Id<PlexilLibNodeCallBody>::convertable(body))
      toXml((PlexilLibNodeCallBody*) body, retval);
    else
      checkParserException(realBody != NULL, "Unknown body type.");

    // *** FIXME: add source locators to called fns above ***
    addSourceLocators(retval, body);
  }

  void PlexilXmlParser::toXml(const PlexilVarRef* ref, xml_node& parent)
    throw(ParserException) 
  {
    if (Id<PlexilInternalVar>::convertable(ref->getId()))
      toXml((PlexilInternalVar*) ref, parent);
    else
      appendNamedTextElement((ref->typed() ? valueTypeString(ref->type()) + VAR_TAG : VAR_TAG).c_str(),
                             ref->name().c_str(),
                             parent);
  }

  void PlexilXmlParser::toXml(const PlexilOp* op, xml_node& parent) 
    throw(ParserException) 
  {
    xml_node retval = appendElement(op->name().c_str(), parent);
    for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin(); 
         it != op->subExprs().end();
         ++it)
      toXml(*it, retval);
  }

  void PlexilXmlParser::toXml(const PlexilArrayElement* op, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(ARRAYELEMENT_TAG, parent);
    appendNamedTextElement(NAME_TAG, op->getArrayName().c_str(), retval);
    for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin(); 
         it != op->subExprs().end();
         ++it)
      toXml(*it, retval);
  }

  // *** FIXME: this violates the current LookupOnChange schema ***
  void PlexilXmlParser::toXml(const PlexilLookup* lookup, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval;
    if (Id<PlexilLookupNow>::convertable(lookup->getId()))
      retval = appendElement(LOOKUPNOW_TAG, parent);
    else if (Id<PlexilChangeLookup>::convertable(lookup->getId()))
      retval = toXml((PlexilChangeLookup*) lookup, parent);
    else
      checkParserException(ALWAYS_FAIL, "Unknown lookup type.");
    toXml(lookup->state(), retval);
  }

  // *** FIXME: this violates the current LookupOnChange schema ***
  xml_node PlexilXmlParser::toXml(const PlexilChangeLookup* lookup, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(LOOKUPCHANGE_TAG, parent);
    for (std::vector<PlexilExprId>::const_iterator it = lookup->tolerances().begin(); 
         it != lookup->tolerances().end();
         ++it)
      toXml(*it, retval);
    return retval;
  }

  void PlexilXmlParser::toXml(const PlexilValue* val, xml_node& parent)
    throw(ParserException) 
  {
    appendNamedTextElement((valueTypeString(val->type()) + VAL_TAG).c_str(),
                           val->value().c_str(),
                           parent);
  }

  void PlexilXmlParser::toXml(const PlexilListBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(NODELIST_TAG, parent);
    for (std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
         it != body->children().end();
         ++it)
      toXml(*it, retval);
  }

  void PlexilXmlParser::toXml(const PlexilUpdateBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(UPDATE_TAG, parent);
    toXml(body->update(), retval);
  }

  void PlexilXmlParser::toXml(const PlexilAssignmentBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(ASSN_TAG, parent);
    for (std::vector<PlexilExpr*>::const_iterator it = body->dest().begin();
         it != body->dest().end();
         ++it)
      toXml(*it, retval);
    xml_node rhs = appendElement(RHS_TAG, retval);
    toXml(body->RHS(), rhs);
  }

  void PlexilXmlParser::toXml(const PlexilCommandBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(CMD_TAG, parent);
    // *** FIXME: add resource list! ***
    for (std::vector<PlexilExpr*>::const_iterator it = body->dest().begin();
         it != body->dest().end();
         ++it)
      toXml(*it, retval);
    toXml(body->state(), retval);
  }

  void PlexilXmlParser::toXml(const PlexilLibNodeCallBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(LIBRARYNODECALL_TAG, parent);
    appendNamedTextElement(NODEID_TAG, body->libNodeName().c_str(), retval);

    // format variable aliases
    for (PlexilAliasMap::const_iterator it = body->aliases().begin();
         it != body->aliases().end(); 
         ++it) {
      // double is key to LabelStr of formal param name
      // expr is actual param
      const std::pair<double, PlexilExprId>& entry = *it;
      xml_node aliasXml = appendElement(ALIAS_TAG, retval);
      appendNamedTextElement(NODE_PARAMETER_TAG, LabelStr(entry.first).c_str(), aliasXml);
      toXml(entry.second, aliasXml);
    }

    // linked library node currently ignored
  }

  void PlexilXmlParser::toXml(const PlexilInternalVar* var, xml_node& parent)
    throw(ParserException) 
  {
    string name(NODE_TAG);
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
    xml_node retval = appendElement(name.c_str(), parent);
    toXml(var->ref(), retval);

    if (Id<PlexilTimepointVar>::convertable(var->getId())) {
      PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
      appendNamedTextElement(STATEVAL_TAG, tp->state().c_str(), retval);
      appendNamedTextElement(TIMEPOINT_TAG, tp->timepoint().c_str(), retval);
    }
  }

  void PlexilXmlParser::toXml(const PlexilStateId& state, xml_node& parent)
    throw(ParserException) 
  {
    appendNamedTextElement(NAME_TAG, state->name().c_str(), parent);
    xml_node args = appendElement(ARGS_TAG, parent);
    for (std::vector<PlexilExprId>::const_iterator it = state->args().begin();
         it != state->args().end();
         ++it)
      toXml(*it, args);
  }

  void PlexilXmlParser::toXml(const PlexilUpdateId& update, xml_node& parent)
    throw(ParserException) 
  {
    for (std::vector<std::pair<string, PlexilExprId> >::const_iterator it = update->pairs().begin(); 
         it != update->pairs().end();
         ++it) {
      xml_node pair = appendElement(PAIR_TAG, parent);
      appendNamedTextElement(NAME_TAG, it->first.c_str(), pair);
      toXml(it->second, pair);
    }
  }

  void PlexilXmlParser::toXml(const PlexilNodeRefId& ref, xml_node& parent)
    throw(ParserException) 
  {
    const char* dir = NULL;
    xml_node retval;
    switch (ref->dir()) {
    case PlexilNodeRef::SELF:
      dir = "self";
      retval = appendElement(NODEREF_TAG, parent);
      break;
    case PlexilNodeRef::PARENT:
      dir = "parent";
      retval = appendElement(NODEREF_TAG, parent);
      break;
    case PlexilNodeRef::CHILD:
      dir = "child";
      retval = appendNamedTextElement(NODEREF_TAG, ref->name().c_str(), parent);
      break;
    case PlexilNodeRef::SIBLING:
      dir = "sibling";
      retval = appendNamedTextElement(NODEREF_TAG, ref->name().c_str(), parent);
      break;
    default:
      checkParserException(ALWAYS_FAIL, "Unknown direction " << ref->dir());
      break;
    }

    // Shouldn't happen, but...
    checkParserException(dir != NULL, "Internal error: dir == NULL");
    checkParserException(retval, "Internal error: retval is empty");

    retval.append_attribute(DIR_ATTR).set_value(dir);
    addSourceLocators(retval, ref);
  }
}
