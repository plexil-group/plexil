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

#include "PlexilXmlParser.hh"

#include "Debug.hh"
#include "Error.hh"
#include "Node.hh"
#include "PlexilPlan.hh"
#include "PlexilSchema.hh"
#include "ValueType.hh"
#include "lifecycle-utils.h"
#include "resource-tags.hh"

#include <cstring>
#include <set>
#include <sstream>

using std::string;
using namespace pugi;

namespace PLEXIL 
{
  // Abstract base class for element parsers
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

  //
  // Parser tables
  //
  static std::map<string, PlexilExprParser*> exprParsers;
  static std::map<string, PlexilBodyParser*> bodyParsers;

  //
  // Internal error checking/reporting utilities
  //

  static inline bool testTag(const char* t, const xml_node& e) {
    return e.type() == node_element && 0 == strcmp(t, e.name());
  }

  static bool testTagPrefix(const char* prefix, const xml_node& e)
  {
    if (e.type() != node_element)
      return false;
    const char* valueStr = e.name();
    const size_t prefixLen = strlen(prefix);
    if (strlen(valueStr) < prefixLen)
      return false;
    return 0 == strncmp(prefix, valueStr, prefixLen);
  }

  static bool testTagSuffix(const char* suffix, const xml_node& e)
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

  static inline bool hasChildElement(const xml_node& e) 
  {
    return e && e.first_child() && e.first_child().type() == node_element;
  }

  static void checkTag(const char* t, const xml_node& e) {
    checkParserExceptionWithLocation(testTag(t, e),
                                     e,
                                     "XML parsing error: Expected <" << t << "> element, but got <" << e.name() << "> instead.");
  }

  static void checkAttr(const char* t, const xml_node& e) {
    checkParserExceptionWithLocation(e && e.type() == node_element && e.attribute(t),
                                     e,
                                     "XML parsing error: Expected an attribute named '" << t << "' in element <" << e.name() << ">");
  }

  // Apparently not used.
  // static void checkTagPrefix(const char* t, const xml_node& e) 
  // {
  //   checkParserExceptionWithLocation(testTagPrefix(t, e),
  //                                    e,
  //                                    "XML parsing error: Expected an element starting in '" << t << "', but instead got <" << e.name() << ">");
  // }

  static void checkTagSuffix(const char* t, const xml_node& e) 
  {
    checkParserExceptionWithLocation(testTagSuffix(t, e),
                                     e,
                                     "XML parsing error: Expected an element ending in '" << t << "', but instead got <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  static void checkNotEmpty(const xml_node& e) {
    checkParserExceptionWithLocation(e.first_child()
                                     && e.first_child().type() == node_pcdata
                                     && *(e.first_child().value()),
                                     e,
                                     "XML parsing error: Expected a non-empty text child of <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  static void checkHasChildElement(const xml_node& e) {
    checkParserExceptionWithLocation(hasChildElement(e),
                                     e,
                                     "XML parsing error: Expected a child element of <" << e.name() << ">");
  }

  static bool isBoolean(const char* initval)
  {
    if (initval == NULL)
      return false;

    switch (*initval) {
    case '0':
    case '1':
      if (*++initval)
        return false;
      else
        return true;

    case 'f':
      return (0 == strcmp(++initval, "alse"));

    case 't':
      return (0 == strcmp(++initval, "rue"));

    default:
      return false;
    }
  }

  static bool isInteger(const char* initval)
  {
    if (initval == NULL || !*initval)
      return false;

    // Check against XML 'integer'
    // [\-+]?[0-9]+
    if ('+' == *initval || '-' == *initval) {
      if (!*++initval)
        return false; // sign w/ nothing after it
    }
    if (isdigit(*initval)) {
      while (*++initval && isdigit(*initval)) {
      }
    }
    if (*initval)
      return false; // junk after number

    // TODO: add range check
    return true;
  }

  static bool isDouble(const char* initval)
  {
    if (initval == NULL || !*initval)
      return false;
      
    // Check against XML 'double'
    // (\+|-)?([0-9]+(\.[0-9]*)?|\.[0-9]+)([Ee](\+|-)?[0-9]+)?|(\+|-)?INF|NaN

    if (strcmp(initval, "NaN") == 0) 
      return true;

    if ('+' == *initval || '-' == *initval) {
      if (!*++initval)
        return false; // sign w/ nothing after it
    }

    if (*initval && strcmp(initval, "INF") == 0)
      return true;

    // ([0-9]+(\.[0-9]*)?|\.[0-9]+)([Ee](\+|-)?[0-9]+)?
    bool digitsSeen = false;
    if (*initval && isdigit(*initval)) {
      digitsSeen = true;
      while (*++initval && isdigit(*initval)) {
      }
    }
    if (*initval && '.' == *initval) {
      if (*++initval && isdigit(*initval)) {
        digitsSeen = true;        
        while (*++initval && isdigit(*initval)) {
        }
      }
    }
    if (!digitsSeen)
      return false;

    // Optional exponent
    if (*initval && ('E' == *initval || 'e' == *initval)) {
      if (!*++initval)
        return false; 
      if ('+' == *initval || '-' == *initval) {
        if (!*++initval)
          return false; 
      }
      if (isdigit(*initval)) {
        while (*++initval && isdigit(*initval)) {
        }
      }
    }

    if (*initval)
      return false; // junk after number

    // FIXME: add range check?
    return true;
  }
  
  static bool isValidConditionName(const string& name)
  {
    return name == START_CONDITION_TAG
      || name == REPEAT_CONDITION_TAG
      || name == PRE_CONDITION_TAG
      || name == POST_CONDITION_TAG
      || name == INVARIANT_CONDITION_TAG
      || name == END_CONDITION_TAG
      || name == EXIT_CONDITION_TAG
      || name == SKIP_CONDITION_TAG;
  }

  // returns the first XML element up the tree with a tag of NODE_TAG
  static xml_node getNodeParent(const xml_node& node) 
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

  static PlexilNodeRef *getLocalNodeRef(const char* name,
                                        const xml_node& node,
                                        const xml_node& referringNode,
                                        const xml_node& ref)
    throw(ParserException)
  {
    checkTag(NODE_TAG, node);

    bool foundSelf = false;
    xml_node nodeId = node.child(NODEID_TAG);
    if (nodeId && (nodeId = nodeId.first_child())
        && 0 == strcmp(name, nodeId.value())) {
      debugMsg("PlexilXmlParser:getNodeRef", " name matches self");
      foundSelf = true;
    }

    // Check this node's children, if any
    xml_node checkChild = node.child(BODY_TAG);
    if (checkChild && (checkChild = checkChild.child(NODELIST_TAG))) {
      bool foundChild = false;
      for (checkChild = checkChild.child(NODE_TAG);
           checkChild;
           checkChild = checkChild.next_sibling(NODE_TAG)) {
        if (checkChild != referringNode) {
          nodeId = checkChild.child(NODEID_TAG);
          if (nodeId && (nodeId = nodeId.first_child())
              && 0 == strcmp(name, nodeId.value())) {
            // FIXME: has the duplicate naming error been discovered by now?
            checkParserExceptionWithLocation(!(foundSelf || foundChild),
                                             checkChild,
                                             "Node and its "
                                             << (foundSelf ? "child" : "sibling")
                                             << " have same node id \"" << name << "\"");
            debugMsg("PlexilXmlParser:getNodeRef", " name matches child");
            foundChild = true;
          }
        }
      }
      if (foundChild)
        return new PlexilNodeRef(PlexilNodeRef::CHILD, name);
    }

    if (foundSelf)
      return new PlexilNodeRef(PlexilNodeRef::SELF, name);
    else
      return NULL;
  }

  static PlexilNodeRef *getNodeRefInternal(const char* name,
                                           const xml_node& node,
                                           const xml_node& referringNode,
                                           const xml_node& ref)
    throw(ParserException) 
  {
    PlexilNodeRef *result = getLocalNodeRef(name, node, referringNode, ref);
    if (result)
      return result;

    // Check ancestors
    xml_node parent = getNodeParent(node);
    int generation = 1;
    while (parent) {
      debugMsg("PlexilXmlParser:getNodeRef", " checking parent node");
      result = getLocalNodeRef(name, parent, referringNode, ref);
      if (result) {
        result->setGeneration(generation);
        return result;
      }
      parent = getNodeParent(parent);
      generation++;
    }

    return NULL;
  }

  static PlexilNodeRef *getNodeRef(const xml_node& ref,
                                   const xml_node& node)
    throw(ParserException) 
  {
    checkTag(NODEID_TAG, ref);
    const char* name = ref.first_child().value();

    debugMsg("PlexilXmlParser:getNodeRef", " for \"" << name << "\"");

    PlexilNodeRef *result = getNodeRefInternal(name, node, node, ref);
    checkParserExceptionWithLocation(result,
                                     ref.first_child(),
                                     "Node '" << node.child(NODEID_TAG).first_child().value()
                                     << "' is trying to access node '" << name
                                     << "' which is out of scope or does not exist");

    return result;
  }

  static PlexilNodeRef *parseNodeRef(const xml_node& ref)
  throw(ParserException) {
    checkTag(NODEREF_TAG, ref);
    checkAttr(DIR_ATTR, ref);
    PlexilNodeRef::Direction dir = PlexilNodeRef::NO_DIR;
    const char* dirValue = ref.attribute(DIR_ATTR).value();
    if (0 == strcmp(dirValue, SELF_VAL))
      return new PlexilNodeRef(PlexilNodeRef::SELF);
    else if (0 == strcmp(dirValue, PARENT_VAL))
      return new PlexilNodeRef(PlexilNodeRef::PARENT);
    else if (0 == strcmp(dirValue, CHILD_VAL))
      dir = PlexilNodeRef::CHILD;
    else if (0 == strcmp(dirValue, SIBLING_VAL))
      dir = PlexilNodeRef::SIBLING;
    else {
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       ref,
                                       "XML parsing error: Invalid value for " << DIR_ATTR << " attibute \""
                                       << dirValue << "\"");
    }
    checkNotEmpty(ref);
    return new PlexilNodeRef(dir, ref.first_child().value());
  }

  static PlexilNodeRef *parseNodeReference(const xml_node& xml) 
  {
    xml_node child = xml.child(NODEID_TAG);
    //if we have an old-style node reference, we have to do a lot of work!
    if (child != NULL)
      return getNodeRef(child, getNodeParent(xml));
    else if ((child = xml.child(NODEREF_TAG)))
      return parseNodeRef(child);
    else {
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       xml,
                                       "XML parsing error: Internal variable reference lacks "
                                       << NODEID_TAG << " or "
                                       << NODEREF_TAG << " tag");

      return NULL;
    }
  }

  static PlexilExpr *parseExpr(const xml_node& xml)
    throw(ParserException) 
  {
    std::map<string, PlexilExprParser*>::iterator it =
      exprParsers.find(std::string(xml.name()));
    checkParserExceptionWithLocation(it != exprParsers.end(),
                                     xml,
                                     "XML parsing error: No parser for expression '" << xml.name() << "'");
    return it->second->parse(xml);
  }

  // N.B. Tolerance comes between Name and Arguments in a LookupOnChange.
  // N.B. Also used by Command parser, where position of Name is flexible (may be 1st - 3rd element!).
  static PlexilState *parseState(const xml_node& xml)
  throw(ParserException) 
  {
    PlexilState *retval = new PlexilState();
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
        retval->addArg(parseExpr(child));
      }
    }

    return retval;
  }

  static ValueType parseValueTypePrefix(const std::string & str, 
                                        std::string::size_type prefixLen)
  {
    switch (prefixLen) {
    case 4: 
      if (0 == str.compare(0, prefixLen, REAL_STR))
        return PLEXIL::REAL_TYPE;
      else if (0 == str.compare(0, prefixLen, DATE_STR))
        return PLEXIL::DATE_TYPE;
      else 
        return PLEXIL::UNKNOWN_TYPE;

    case 5:
      if (0 == str.compare(0, prefixLen, ARRAY_STR))
        return PLEXIL::ARRAY_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 6:
      if (0 == str.compare(0, prefixLen, STRING_STR))
        return PLEXIL::STRING_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 7:
      if (0 == str.compare(0, prefixLen, INTEGER_STR))
        return PLEXIL::INTEGER_TYPE;
      else if (0 == str.compare(0, prefixLen, BOOL_STR))
        return PLEXIL::BOOLEAN_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 8:
      if (0 == str.compare(0, prefixLen, DURATION_STR))
        return PLEXIL::DURATION_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;


    case 9:
      if (0 == str.compare(0, prefixLen, NODE_STATE_STR))
        return PLEXIL::NODE_STATE_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 11:
      if (0 == str.compare(0, prefixLen, NODE_OUTCOME_STR))
        return PLEXIL::OUTCOME_TYPE;
      else if (0 == str.compare(0, prefixLen, NODE_FAILURE_STR))
        return PLEXIL::FAILURE_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 17:
      if (0 == str.compare(0, prefixLen, NODE_COMMAND_HANDLE_STR))
        return PLEXIL::COMMAND_HANDLE_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;
      
      // default case
    default:
      return PLEXIL::UNKNOWN_TYPE;
    }
  }

  inline static ValueType parseValueType(const std::string& typeStr)
  {
    return parseValueTypePrefix(typeStr, typeStr.length());
  }

  // parse an array declaration

  static PlexilVar* parseArrayDeclaration(const xml_node& decl)
    throw(ParserException) {
    checkTag(DECL_ARRAY_TAG, decl);

    // extract array name

    xml_node child = decl.first_child();
    checkTag(NAME_TAG, child);
    const char* name = child.first_child().value();

    checkParserExceptionWithLocation(name != Node::STATE()
                                     && name != Node::OUTCOME()
                                     && name != Node::FAILURE_TYPE(),
                                     child.first_child(),
                                     "In array variable declaration: name \"" << name << "\" is reserved");

    // extract array type

    child = child.next_sibling();
    checkTag(TYPE_TAG, child);
    const char* typnam = child.first_child().value();
    ValueType typ = parseValueType(typnam);
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

    PlexilVar *result = new PlexilArrayVar(name, typ, maxSize, initVals);
    debugMsg("PlexilXmlParser:parseArrayDeclaration", " succeeded");
    return result;
  }

  // parse an atomic or string declaration

  static PlexilVar *parseAtomicOrStringDeclaration(const xml_node& decl) 
    throw(ParserException) 
  {
    checkTag(DECL_VAR_TAG, decl);

    // extract name
    xml_node child = decl.first_child();
    checkTag(NAME_TAG, child);
    const string& name = child.first_child().value();

    checkParserExceptionWithLocation(name != Node::STATE()
                                     && name != Node::OUTCOME()
                                     && name != Node::FAILURE_TYPE(),
                                     child.first_child(),
                                     "In variable declaration: name \"" << name << "\" is reserved");

    // extract type
    child = child.next_sibling();
    checkTag(TYPE_TAG, child);
    const char* typnam = child.first_child().value();
    ValueType typ = parseValueType(typnam);
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     child.first_child(),
                                     "Unknown type name \"" << typnam << "\"");

    // if present, create variable with initial value
    if ((child = child.next_sibling())) {
      checkTag(INITIALVAL_TAG, child);
      child = child.first_child();
      checkTagSuffix(VAL_TAG, child);
      // FIXME: check for junk between checked prefix and suffix of tag
      checkParserExceptionWithLocation(testTagPrefix(typnam, child),
                                       child,
                                       "Initial value of " << typnam << " variable \'" <<
                                       name << "\' of incorrect type \'" << child.name() << "\'");

      // Check value
      checkParserExceptionWithLocation(typ == STRING_TYPE || *(child.first_child().value()),
                                       child.first_child(),
                                       "Empty initial value is not valid for " << typnam << " variable \'" <<
                                       name << "\'");
      const char* initval = child.first_child().value();
      if (typ == BOOLEAN_TYPE) {
        checkParserExceptionWithLocation(isBoolean(initval),
                                         child.first_child(),
                                         "Invalid Boolean initial value \"" << initval << "\" for variable \'" << name << "\'");
      }
      else if (typ == INTEGER_TYPE) {
        checkParserExceptionWithLocation(isInteger(initval),
                                         child.first_child(),
                                         "Invalid Integer initial value \"" << initval << "\" for variable \'" << name << "\'");
      }
      else if (typ == REAL_TYPE) {
        checkParserExceptionWithLocation(isDouble(initval),
                                         child.first_child(),
                                         "Invalid Real initial value \"" << initval << "\" for variable \"" << name << "\"");
      }

      return new PlexilVar(name, typ, child.first_child().value());
    }

    // otherwise create variable with the value unknown
    return new PlexilVar(name, typ);
  }

  static PlexilVar* parseDeclaration(const xml_node& decl)
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

  static void parseDeclarations(const xml_node& decls,
                                PlexilNode *node)
    throw(ParserException) 
  {
    checkTag(VAR_DECLS_TAG, decls);
    for (xml_node decl = decls.first_child();
         decl;
         decl = decl.next_sibling())
      node->addVariable(parseDeclaration(decl));
  }

  static void parseInOrInOut(const xml_node& inOrInOut,
                             PlexilInterface *interface, 
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
        PlexilVar *variable = parseDeclaration(var);

        // convert variable to var ref
        PlexilVarRef *varRef = new PlexilVarRef(variable->varName(), variable->type());
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

  static PlexilInterface *parseInterface(const xml_node& intf)
    throw(ParserException) {
    PlexilInterface *retval = new PlexilInterface();
    checkTag(INTERFACE_TAG, intf);
    parseInOrInOut(intf.child(IN_TAG), retval, false);
    parseInOrInOut(intf.child(INOUT_TAG), retval, true);
    return retval;
  }

  static std::vector<PlexilResource *> parseResource(const xml_node& xml) 
    throw(ParserException) 
  {
    std::vector<PlexilResource *> rId_vec;

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

        PlexilResource *prId = new PlexilResource();
        for (xml_node child3 = resourceElt.first_child(); 
             child3; 
             child3 = child3.next_sibling()) {
          // add each resource element just like addArg to PLexilResourceId. Use
          // tag3 and expresssion the in <name, expr> pair
          prId->addResource(child3.name(),
                            parseExpr(child3.first_child()));
        }

        // push the PlexilResourceId into a vector to be returned and
        // used in the PlexilCommandBody.
        rId_vec.push_back(prId);
      }
    }

    // return the vector of PlexilResourceId
    return rId_vec;
  }

  static PlexilNodeBody *parseBody(const xml_node& body)
  throw(ParserException)
  {
    const char* name = body.name();
    std::map<string, PlexilBodyParser*>::iterator it = bodyParsers.find(string(name));
    checkParserExceptionWithLocation(it != bodyParsers.end(),
                                     body,
                                     "XML parsing error: No parser for body type \"" << name << "\"");
    return it->second->parse(body);
  }

  static PlexilNode *parseNode(const xml_node& xml)
  throw(ParserException) 
  {
    checkTag(NODE_TAG, xml);
    PlexilNode *retval = new PlexilNode();

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
                                       "XML parsing error: " << nodeTypeString(retval->nodeType())
                                       << " node \"" << retval->nodeId() <<
                                       "\" lacks a <NodeBody> element.");
    }
    else {
      checkParserExceptionWithLocation(retval->nodeType() != NodeType_Empty,
                                       bodyXml,
                                       "XML parsing error: Empty node \"" << retval->nodeId()
                                       << "\" may not contain a <" << BODY_TAG << "> element.");

      // Get real body
      xml_node realBodyXml = bodyXml.first_child();
      checkParserExceptionWithLocation(realBodyXml && realBodyXml.type() == node_element,
                                       bodyXml,
                                       "XML parsing error: Node \"" << retval->nodeId()
                                       << "\": <NodeBody> element improperly formatted");

      PlexilNodeBody *bodyStruct = parseBody(realBodyXml);

      // Check that body is of correct type
      switch (retval->nodeType()) {
      case NodeType_NodeList:
        // check for PlexilListBody
        checkParserExceptionWithLocation(NULL != (PlexilListBody*) bodyStruct,
                                         realBodyXml,
                                         "XML parsing error: Body of " << nodeTypeString(retval->nodeType())
                                         << " node \"" << retval->nodeId()
                                         << "\" contains a " << realBodyXml.name() << " element.")
          break;

      case NodeType_Command:
        // check for PlexilCommandBody
        checkParserExceptionWithLocation(NULL != (PlexilCommandBody*) bodyStruct,
                                         realBodyXml,
                                         "XML parsing error: Body of " << nodeTypeString(retval->nodeType())
                                         << " node \"" << retval->nodeId()
                                         << "\" contains a " << realBodyXml.name() << " element.")
          break;

      case NodeType_Assignment:
        // check for PlexilAssignmentBody
        checkParserExceptionWithLocation(NULL != (PlexilAssignmentBody*) bodyStruct,
                                         realBodyXml,
                                         "XML parsing error: Body of " << nodeTypeString(retval->nodeType())
                                         << " node \"" << retval->nodeId()
                                         << "\" contains a " << realBodyXml.name() << " element.")
          break;

      case NodeType_Update:
        // check for PlexilUpdateBody
        checkParserExceptionWithLocation(NULL != (PlexilUpdateBody*) bodyStruct,
                                         realBodyXml,
                                         "XML parsing error: Body of " << nodeTypeString(retval->nodeType())
                                         << " node \"" << retval->nodeId()
                                         << "\" contains a " << realBodyXml.name() << " element.")
          break;

      case NodeType_LibraryNodeCall:
        // check for PlexilLibNodeCallBody
        checkParserExceptionWithLocation(NULL != (PlexilLibNodeCallBody*) bodyStruct,
                                         realBodyXml,
                                         "XML parsing error: Body of " << nodeTypeString(retval->nodeType())
                                         << " node \"" << retval->nodeId()
                                         << "\" contains a " << realBodyXml.name() << " element.")
          break;

      default:
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         xml,
                                         "XML parser internal error: Invalid node type while parsing node body");
        break;
      }
      retval->setBody(bodyStruct);
    }

    return retval;
  }

  //
  // Implementation of parser class methods
  //

  class PlexilOutcomeVarParser: public PlexilExprParser {
  public:
    PlexilOutcomeVarParser() : PlexilExprParser() {}
    ~PlexilOutcomeVarParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      return new PlexilOutcomeVar(parseNodeReference(xml));
    }
  };

  class PlexilFailureVarParser: public PlexilExprParser {
  public:
    PlexilFailureVarParser() : PlexilExprParser() {}
    ~PlexilFailureVarParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      return new PlexilFailureVar(parseNodeReference(xml));
    }
  };

  class PlexilStateVarParser: public PlexilExprParser {
  public:
    PlexilStateVarParser() : PlexilExprParser() {}
    ~PlexilStateVarParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      return new PlexilStateVar(parseNodeReference(xml));
    }
  };

  class PlexilCommandHandleVarParser: public PlexilExprParser {
  public:
    PlexilCommandHandleVarParser() : PlexilExprParser() {}
    ~PlexilCommandHandleVarParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      return new PlexilCommandHandleVar(parseNodeReference(xml));
    }
  };

  class PlexilTimepointVarParser: public PlexilExprParser {
  public:
    PlexilTimepointVarParser() : PlexilExprParser() {}
    ~PlexilTimepointVarParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      PlexilNodeRef *nodeRef = parseNodeReference(xml);
      xml_node stateElt = xml.child(STATEVAL_TAG);
      checkParserExceptionWithLocation(stateElt,
                                       xml,
                                       "XML parsing error: Timepoint missing " << STATEVAL_TAG << " tag");
      checkNotEmpty(stateElt);
      std::string state(stateElt.first_child().value());
      xml_node pointElt = xml.child(TIMEPOINT_TAG);
      checkParserExceptionWithLocation(pointElt,
                                       xml,
                                       "XML parsing error: Timepoint missing " << TIMEPOINT_TAG << " tag");
      checkNotEmpty(pointElt);
      std::string timept(pointElt.first_child().value());

      return new PlexilTimepointVar(nodeRef, state, timept);
    }
  };

  class PlexilOpParser: public PlexilExprParser {
  public:
    PlexilOpParser() : PlexilExprParser() {}
    ~PlexilOpParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      PlexilOp* retval = new PlexilOp(xml.name());
      for (xml_node child = xml.first_child(); 
           child;
           child = child.next_sibling())
        retval->addSubExpr(parseExpr(child));
      return retval;
    }
  };

  class PlexilChangeLookupParser: public PlexilExprParser {
  public:
    PlexilChangeLookupParser() : PlexilExprParser() {}
    ~PlexilChangeLookupParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) {
      checkTag(LOOKUPCHANGE_TAG, xml);
      PlexilChangeLookup* retval = new PlexilChangeLookup();
      retval->setState(parseState(xml));
      retval->setName("LookupOnChange");
      xml_node tol = xml.child(TOLERANCE_TAG); 
      if (tol) {
        checkHasChildElement(tol);
        retval->setTolerance(parseExpr(tol.first_child()));
      }
      return retval;
    }
  };

  class PlexilLookupNowParser: public PlexilExprParser {
  public:
    PlexilLookupNowParser() : PlexilExprParser() {}
    ~PlexilLookupNowParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) {
      checkTag(LOOKUPNOW_TAG, xml);
      PlexilLookup* retval = new PlexilLookup();
      retval->setName("LookupNow");
      retval->setState(parseState(xml));
      return retval;
    }
  };

  class PlexilArrayElementParser: public PlexilExprParser {
  public:
    PlexilArrayElementParser() : PlexilExprParser() {}
    ~PlexilArrayElementParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) {
      checkTag(ARRAYELEMENT_TAG, xml);

      // parse the array parameter
      xml_node ary = xml.first_child();
      PlexilExpr *aryExpr;
      if (testTag(NAME_TAG, ary))
        // Old style - create an array variable reference
        aryExpr = new PlexilVarRef(ary.first_child().value(),
                                   ARRAY_TYPE); // ??
      else 
        aryExpr = parseExpr(ary);

      // extract index
      xml_node idx = ary.next_sibling();
      checkTag(INDEX_TAG, idx);
      PlexilExpr *indexExpr = parseExpr(idx.first_child());

      return new PlexilArrayElement(aryExpr, indexExpr);
    }
  };

  class PlexilValueParser: public PlexilExprParser {
  public:
    PlexilValueParser() : PlexilExprParser() {}
    ~PlexilValueParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) 
    {
      // confirm that we have a value
      checkTagSuffix(VAL_TAG, xml);

      // establish value type
      const char* tag = xml.name();
      ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_TAG));
      checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                       xml,
                                       "Unrecognized value type \"" << tag << "\"");

      // check for empty value
      if (!xml.first_child() || !*(xml.first_child().value())) {
        checkParserExceptionWithLocation(typ == STRING_TYPE,
                                         xml.first_child(),
                                         "Empty value is not valid for \"" << tag << "\"");
        return new PlexilValue(typ, string());
      }

      // Check value format
      const char* initval = xml.first_child().value();
      if (typ == BOOLEAN_TYPE) {
        checkParserExceptionWithLocation(isBoolean(initval),
                                         xml.first_child(),
                                         "Invalid Boolean value \"" << initval << "\"");
      }
      else if (typ == INTEGER_TYPE) {
        checkParserExceptionWithLocation(isInteger(initval),
                                         xml.first_child(),
                                         "Invalid Integer value \"" << initval << "\"");
      }
      else if (typ == REAL_TYPE) {
        checkParserExceptionWithLocation(isDouble(initval),
                                         xml.first_child(),
                                         "Invalid Real value \"" << initval << "\"");
      }

      // return new (non-empty) value
      return new PlexilValue(typ, xml.first_child().value());
    }
  };

  class PlexilArrayValueParser: public PlexilExprParser {
  public:
    PlexilArrayValueParser() : PlexilExprParser() {}
    ~PlexilArrayValueParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) {
      // confirm that we have an array value
      checkTag(ARRAY_VAL_TAG, xml);

      // confirm that we have an element type
      checkAttr(TYPE_TAG, xml);
      const char* valueType = xml.attribute(TYPE_TAG).value();
      ValueType valtyp = parseValueType(valueType);
      checkParserExceptionWithLocation(valtyp != UNKNOWN_TYPE,
                                       xml, // *** should be the attribute object
                                       "Unknown array element Type value \"" << valueType << "\"");

      // gather elements
      std::vector<string> values;

      xml_node thisElement = xml.first_child();
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
        thisElement = thisElement.next_sibling();
      }

      // return new value
      return new PlexilArrayValue(valtyp, values.size(), values);
    }
  };

  class PlexilVarRefParser: public PlexilExprParser {
  public:
    PlexilVarRefParser() : PlexilExprParser() {}
    ~PlexilVarRefParser() {}

    PlexilExpr *parse(const xml_node& xml) throw(ParserException) {
      checkTagSuffix(VAR_TAG, xml);
      checkNotEmpty(xml);
      const char* tag = xml.name();
      ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAR_TAG));
      checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                       xml,
                                       "Unknown variable type \"" << tag << "\"");

      return new PlexilVarRef(xml.first_child().value(), typ);
    }
  };

  class PlexilActionParser: public PlexilBodyParser {
  public:
    PlexilActionParser() : PlexilBodyParser() {}
    virtual ~PlexilActionParser() {}

    // FIXME: Limit to one destination!
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

    PlexilNodeBody *parse(const xml_node& xml) throw(ParserException) {
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
      retval->setRHS(parseExpr(rhs.first_child()));
      return retval;
    }
  };

  class PlexilNodeListParser: public PlexilBodyParser {
  public:
    PlexilNodeListParser() : PlexilBodyParser() {}
    ~PlexilNodeListParser() {}

    PlexilNodeBody *parse(const xml_node& xml) throw(ParserException) {
      checkTag(NODELIST_TAG, xml);
      PlexilListBody* retval = new PlexilListBody();
      std::set<string> childIds;
      for (xml_node child = xml.child(NODE_TAG); 
           child;
           child = child.next_sibling(NODE_TAG)) {
        PlexilNode *thisNode = parseNode(child);
        // check for duplicate node ID
        checkParserExceptionWithLocation(childIds.find(thisNode->nodeId()) == childIds.end(),
                                         child,
                                         "XML parsing error: Multiple nodes with node ID \"" << thisNode->nodeId() << "\"");
        childIds.insert(thisNode->nodeId());
        retval->addChild(thisNode);
      }
      return retval;
    }
  };

  // parse a library node call

  class PlexilLibraryNodeCallParser: public PlexilBodyParser {
  public:
    PlexilLibraryNodeCallParser() : PlexilBodyParser() {}
    ~PlexilLibraryNodeCallParser() {}

    PlexilNodeBody *parse(const xml_node& xml) throw(ParserException) {
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
        PlexilExpr *value = parseExpr(libParamXml.next_sibling());

        // add alias to body
        body->addAlias(libParam, value);
      }

      // return lib node call node body
      return body;
    }
  };

  class PlexilCommandParser: public PlexilActionParser {
  public:
    PlexilCommandParser() : PlexilActionParser() {}
    ~PlexilCommandParser() {}

    PlexilNodeBody *parse(const xml_node& xml) throw(ParserException) {
      checkTag(CMD_TAG, xml);
      PlexilCommandBody* retval = new PlexilCommandBody();
      parseDest(xml, retval);
      retval->setState(parseState(xml));
      retval->setResource(parseResource(xml));
      return retval;
    }
  };

  class PlexilPairsParser: public PlexilBodyParser {
  public:
    PlexilPairsParser() : PlexilBodyParser() {}
    virtual ~PlexilPairsParser() {}

    PlexilUpdate *parsePairs(const xml_node& xml) throw(ParserException) {
      PlexilUpdate *retval = new PlexilUpdate();
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
        debugMsg("PlexilXmlParser:parsePairs", "Parsed pair {" << name << ", " << *value << "}");
        retval->addPair(name, parseExpr(value));
      }
      return retval;
    }
  };

  class PlexilUpdateParser: public PlexilPairsParser {
  public:
    PlexilUpdateParser() : PlexilPairsParser() {}
    ~PlexilUpdateParser() {} 

    PlexilNodeBody *parse(const xml_node& xml) throw(ParserException) {
      checkTag(UPDATE_TAG, xml);
      PlexilUpdateBody* retval = new PlexilUpdateBody();
      if (xml.child(PAIR_TAG))
        retval->setUpdate(parsePairs(xml));
      return retval;
    }
  };

  //
  // Parser table setup, cleanup
  //

  extern "C"
  void cleanupParserTables()
  {
    for (std::map<string, PlexilBodyParser*>::iterator it = bodyParsers.begin();
         it != bodyParsers.end();
         ++it)
      delete it->second;
    bodyParsers.clear();

    // Have to check for duplicates in expression parsers
    std::set<PlexilExprParser*> parsersToDelete;
    for (std::map<string, PlexilExprParser*>::const_iterator it = exprParsers.begin();
         it != exprParsers.end();
         ++it)
      parsersToDelete.insert(it->second);
    exprParsers.clear();

    for (std::set<PlexilExprParser*>::const_iterator it = parsersToDelete.begin();
         it != parsersToDelete.end();
         ++it)
      delete *it;
  }

  static void registerParsers() 
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&cleanupParserTables);
      bodyParsers.insert(std::make_pair(ASSN_TAG, new PlexilAssignmentParser()));
      bodyParsers.insert(std::make_pair(NODELIST_TAG,
                                        new PlexilNodeListParser()));
      bodyParsers.insert(std::make_pair(LIBRARYNODECALL_TAG,
                                        new PlexilLibraryNodeCallParser()));
      bodyParsers.insert(std::make_pair(CMD_TAG, new PlexilCommandParser()));
      bodyParsers.insert(std::make_pair(UPDATE_TAG, new PlexilUpdateParser()));

      PlexilExprParser* varRef = new PlexilVarRefParser();
      exprParsers.insert(std::make_pair(VAR_TAG, varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(INTEGER_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(REAL_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(ARRAY_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(STRING_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(BOOLEAN_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(DATE_TYPE), varRef));
      exprParsers.insert(std::make_pair(typeNameAsVariable(DURATION_TYPE), varRef));

      exprParsers.insert(std::make_pair(typeNameAsVariable(OUTCOME_TYPE),
                                        new PlexilOutcomeVarParser()));
      exprParsers.insert(std::make_pair(typeNameAsVariable(FAILURE_TYPE),
                                        new PlexilFailureVarParser()));
      exprParsers.insert(std::make_pair(typeNameAsVariable(NODE_STATE_TYPE),
                                        new PlexilStateVarParser()));
      exprParsers.insert(std::make_pair(typeNameAsVariable(COMMAND_HANDLE_TYPE),
                                        new PlexilCommandHandleVarParser()));
      exprParsers.insert(std::make_pair(string("NodeTimepoint") + VAL_TAG,
                                        new PlexilTimepointVarParser()));

      PlexilExprParser* val = new PlexilValueParser();
      exprParsers.insert(std::make_pair(typeNameAsValue(INTEGER_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(REAL_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(STRING_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(BOOLEAN_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(DATE_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(DURATION_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(OUTCOME_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(FAILURE_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(NODE_STATE_TYPE), val));
      exprParsers.insert(std::make_pair(typeNameAsValue(COMMAND_HANDLE_TYPE), val));

      exprParsers.insert(std::make_pair(string(ARRAY_VAL_TAG),
                                        new PlexilArrayValueParser()));

      exprParsers.insert(std::make_pair(LOOKUPNOW_TAG,
                                        new PlexilLookupNowParser()));
      exprParsers.insert(std::make_pair(LOOKUPCHANGE_TAG,
                                        new PlexilChangeLookupParser()));
      exprParsers.insert(std::make_pair(ARRAYELEMENT_TAG,
                                        new PlexilArrayElementParser()));

      PlexilExprParser* op = new PlexilOpParser();
      exprParsers.insert(std::make_pair("AND", op));
      exprParsers.insert(std::make_pair("OR", op));
      exprParsers.insert(std::make_pair("XOR", op));
      exprParsers.insert(std::make_pair("NOT", op));
      exprParsers.insert(std::make_pair("Concat", op));
      exprParsers.insert(std::make_pair("IsKnown", op));
      exprParsers.insert(std::make_pair("EQ", op));
      exprParsers.insert(std::make_pair("EQNumeric", op));
      exprParsers.insert(std::make_pair("EQString", op));
      exprParsers.insert(std::make_pair("EQBoolean", op));
      exprParsers.insert(std::make_pair("EQInternal", op));
      exprParsers.insert(std::make_pair("NE", op));
      exprParsers.insert(std::make_pair("NENumeric", op));
      exprParsers.insert(std::make_pair("NEString", op));
      exprParsers.insert(std::make_pair("NEBoolean", op));
      exprParsers.insert(std::make_pair("NEInternal", op));
      exprParsers.insert(std::make_pair("LT", op));
      exprParsers.insert(std::make_pair("LE", op));
      exprParsers.insert(std::make_pair("GT", op));
      exprParsers.insert(std::make_pair("GE", op));
      exprParsers.insert(std::make_pair("ADD", op));
      exprParsers.insert(std::make_pair("SUB", op));
      exprParsers.insert(std::make_pair("MUL", op));
      exprParsers.insert(std::make_pair("DIV", op));
      exprParsers.insert(std::make_pair("MOD", op));
      exprParsers.insert(std::make_pair("MAX", op));
      exprParsers.insert(std::make_pair("MIN", op));
      exprParsers.insert(std::make_pair("SQRT", op));
      exprParsers.insert(std::make_pair("ABS", op));

      sl_inited = true;
    }
  }

  /*
   * @brief Load the named library node from a file on the given path.
   * @param name Name of the node.
   * @param path Vector of places to search for the file.
   * @return The loaded node, or NULL if not found or error.
   */
  PlexilNode *PlexilXmlParser::findLibraryNode(const string& name,
                                               const std::vector<string>& path)
  {
    return findPlan(name, name, path);
  }

  /*
   * @brief Load the named plan from a file on the given path.
   * @param name Name of the node.
   * @param fileName Name of the file, with or without the ".plx" suffix.
   * @param path Vector of places to search for the file.
   * @return The loaded node, or NULL if not found or error.
   */
  PlexilNode *PlexilXmlParser::findPlan(const string& name,
                                         const string& fileName,
                                         const std::vector<string>& path)
  {
    debugMsg("PlexilXmlParser:findPlan", "(\"" << name << "\", \"" << fileName << "\")");
    PlexilNode *result = NULL;
    std::vector<string>::const_iterator it = path.begin();
    string fileNameWithSuffix = fileName;
    // add suffix if lacking
    if (fileName.length() <= 4 ||
        fileName.compare(fileName.length() - 4, 4, ".plx") != 0)
      fileNameWithSuffix += ".plx";

    // Find the first occurrence of the library in this path
    while (!result && it != path.end()) {
      string candidateFile = *it + "/" + fileNameWithSuffix;
      try {
        result = loadPlanNamed(name, candidateFile);
        if (result)
          return result;
      }
      catch (ParserException& p) {
        debugMsg("PlexilXmlParser:findPlan", 
                 " failed due to error in " 
                 << candidateFile
                 << ":\n"
                 << p.what());
        return NULL;
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
      return NULL;
    }
  }

  /*
   * @brief Load the named plan from a file in the given directory.
   * @param name Name of the desired node.
   * @param filename Candidate file for this node.
   * @return The loaded node, or NULL if not found or error.
   */
  PlexilNode *PlexilXmlParser::loadPlanNamed(const string& name, 
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
      return NULL;
    }
    PlexilNode *result = parse(doc.document_element());
    // Check that node has the desired node ID
    checkParserException(0 == name.compare(result->nodeId()),
                         "Error: File " << filename
                         << " contains node ID \"" << result->nodeId()
                         << "\", not \"" << name << "\"");
    debugMsg("PlexilXmlParser:loadPlanNamed",
             " successfully loaded node " << name << " from " << filename);
    return result;
  }

  PlexilNode *PlexilXmlParser::parse(const string& str, bool isFile)
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
    PlexilNode *result = parse(root);
    if (result->fileName().empty())
      result->setFileName(str);
    return result;
  }

  PlexilNode *PlexilXmlParser::parse(const char* text)
  throw(ParserException) 
  {
    // First parse the XML itself
    xml_document doc;
    xml_parse_result parseResult = doc.load(text, PUGI_PARSE_OPTIONS());
    checkParserException(parseResult.status != status_ok,
                         "(character " << parseResult.offset
                         << ") XML parsing error: " << parseResult.description());

    xml_node root = doc.document_element();
    return parse(root);
  }

  PlexilNode *PlexilXmlParser::parse(xml_node xml)
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
    PlexilNode *result = parseNode(xml);
    // FIXME: Add post-parse checks (e.g. node and variable accessibility) here
    return result;
  }

}
