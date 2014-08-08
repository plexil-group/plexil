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

#include "PlexilToXml.hh"

#include "Error.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"

using namespace pugi;
using std::string;

namespace PLEXIL
{

  //
  // PlexilNode -> XML
  //

  // Helper functions
  static xml_node appendElement(const char* name, xml_node& parent) 
  {
    xml_node retval = parent.append_child();
    retval.set_name(name);
    return retval;
  }

  static xml_node appendNamedTextElement(const char* name,
                                  const char* value, 
                                  xml_node& parent) 
  {
    xml_node retval = parent.append_child();
    retval.set_name(name);
    xml_node text = retval.append_child(node_pcdata);
    text.set_value(value);
    return retval;
  }

  static xml_node appendNamedNumberElement(const char* name,
                                    const double value,
                                    xml_node& parent)
  {
    std::ostringstream str;
    str << value;
    return appendNamedTextElement(name, str.str().c_str(), parent);
  }

  template <typename T>
  static void addSourceLocators(xml_node& xml, T obj)
  {
    int lineno = obj->lineNo();
    if (lineno != 0)
      xml.append_attribute(LINENO_ATTR).set_value(lineno);

    int col = obj->colNo();
    if (col != 0)
      xml.append_attribute(COLNO_ATTR).set_value(col);
  }

  // Main entry point
  xml_document* toXml(PlexilNode const *node)
    throw(ParserException) 
  {
    xml_document* result = new xml_document();
    toXml(node, *result);
    return result;
  }

  void toXml(PlexilNode const *node, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(NODE_TAG, parent);
    retval.append_attribute(NODETYPE_ATTR).set_value(nodeTypeString(node->nodeType()).c_str());
    const string & filename = node->fileName();
    if (!filename.empty())
      retval.append_attribute(FILENAME_ATTR).set_value(filename.c_str());

    appendNamedTextElement(NODEID_TAG, node->nodeId().c_str(), retval);
    appendNamedNumberElement(PRIORITY_TAG, node->priority(), retval);

    addSourceLocators(retval, node);

    if (node->interface())
      toXml(node->interface(), retval);

    if (!node->declarations().empty()) {
      xml_node declarations = appendElement(VAR_DECLS_TAG, retval);
      for (std::vector<PlexilVar *>::const_iterator it = node->declarations().begin(); 
           it != node->declarations().end(); 
           ++it)
        toXml(*it, declarations);
    }

    for (std::vector<std::pair<PlexilExpr *, std::string> >::const_iterator it = node->conditions().begin(); 
         it != node->conditions().end();
         ++it) {
      xml_node cond = appendElement(it->second.c_str(), retval);
      toXml(it->first, cond);
    }

    if (node->body())
      toXml(node->body(), retval);
  }

  void toXml(PlexilInterface const *intf, xml_node& parent)
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

  void toXml(PlexilVar const *var, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement((var->isArray() ? DECL_ARRAY_TAG : DECL_VAR_TAG),
                                    parent);
    appendNamedTextElement(NAME_TAG, var->name().c_str(), retval);
    appendNamedTextElement(TYPE_TAG, valueTypeName(var->type()).c_str(), retval); // FIXME for arrays

    if (var->isArray()) {
      PlexilArrayVar const *arrayVar = dynamic_cast<PlexilArrayVar const *>(var);
      // max size
      appendNamedNumberElement(MAXSIZE_TAG, arrayVar->maxSize(), retval);

      // initial values
      xml_node vals = appendElement(INITIALVAL_TAG, retval);
      std::string const &valueTag = typeNameAsValue(arrayVar->type());
      const std::vector<string>& values =
        ((PlexilArrayValue const *) arrayVar->value())->values();
      for (std::vector<string>::const_iterator it = values.begin();
           it != values.end();
           ++it) 
        appendNamedTextElement(valueTag.c_str(), it->c_str(), vals);
    }
    else if (var->value()) {
      // initial value
      toXml(var->value(), retval);
    }

    addSourceLocators(retval, var);
  }

  void toXml(PlexilExpr const *expr, xml_node& parent)
    throw(ParserException)
  {
    if (dynamic_cast<const PlexilVarRef*> (expr))
      toXml((const PlexilVarRef*) expr, parent);
    else if (dynamic_cast<const PlexilOp*> (expr))
      toXml((const PlexilOp*) expr, parent);
    else if (dynamic_cast<const PlexilArrayElement*> (expr))
      toXml((const PlexilArrayElement*) expr, parent);
    else if (dynamic_cast<const PlexilLookup*> (expr))
      toXml((const PlexilLookup*) expr, parent);
    else if (dynamic_cast<const PlexilValue*> (expr))
      toXml((const PlexilValue*) expr, parent);
    else
      checkParserException(ALWAYS_FAIL, "Should never get here.");

    // *** FIXME: add source locators to called fns above ***
  }

  void toXml(PlexilNodeBody const *body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(BODY_TAG, parent);
    xml_node realBody;
    if (dynamic_cast<PlexilListBody const *>(body))
      toXml((PlexilListBody const *) body, retval);
    else if (dynamic_cast<PlexilUpdateBody const *>(body))
      toXml((PlexilUpdateBody const *) body, retval);
    else if (dynamic_cast<PlexilAssignmentBody const *>(body))
      toXml((PlexilAssignmentBody const *) body, retval);
    else if (dynamic_cast<PlexilCommandBody const *>(body))
      toXml((PlexilCommandBody const *) body, retval);
    else if (dynamic_cast<PlexilLibNodeCallBody const *>(body))
      toXml((PlexilLibNodeCallBody const *) body, retval);
    else
      checkParserException(realBody != NULL, "Unknown body type.");

    // *** FIXME: add source locators to called fns above ***
    addSourceLocators(retval, body);
  }

  void toXml(const PlexilVarRef* ref, xml_node& parent)
    throw(ParserException) 
  {
    if (dynamic_cast<PlexilInternalVar const *>(ref))
      toXml((PlexilInternalVar const *) ref, parent);
    else
      appendNamedTextElement((ref->typed() ? typeNameAsVariable(ref->type()) : VAR_TAG).c_str(),
                             ref->name().c_str(),
                             parent);
  }

  void toXml(const PlexilOp* op, xml_node& parent) 
    throw(ParserException) 
  {
    xml_node retval = appendElement(op->name().c_str(), parent);
    for (std::vector<PlexilExpr *>::const_iterator it = op->subExprs().begin(); 
         it != op->subExprs().end();
         ++it)
      toXml(*it, retval);
  }

  void toXml(const PlexilArrayElement* op, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(ARRAYELEMENT_TAG, parent);
    appendNamedTextElement(NAME_TAG, op->getArrayName().c_str(), retval);
    xml_node idx = appendElement(INDEX_TAG, retval);
    toXml(op->index(), idx);
  }

  // *** FIXME: this violates the current LookupOnChange schema ***
  void toXml(const PlexilLookup* lookup, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval;
    if (dynamic_cast<PlexilChangeLookup const *>(lookup))
      retval = toXml((PlexilChangeLookup const *) lookup, parent);
    else if (dynamic_cast<PlexilLookup const *>(lookup))
      retval = appendElement(LOOKUPNOW_TAG, parent);
    else
      checkParserException(ALWAYS_FAIL, "Unknown lookup type.");
    toXml(lookup->state(), retval);
  }

  // *** FIXME: this violates the current LookupOnChange schema ***
  xml_node toXml(const PlexilChangeLookup* lookup, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(LOOKUPCHANGE_TAG, parent);
    if (lookup->tolerance())
      toXml(lookup->tolerance(), retval);
    return retval;
  }

  void toXml(const PlexilValue* val, xml_node& parent)
    throw(ParserException) 
  {
    appendNamedTextElement(typeNameAsValue(val->type()).c_str(),
                           val->value().c_str(),
                           parent);
  }

  void toXml(const PlexilListBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(NODELIST_TAG, parent);
    for (std::vector<PlexilNode *>::const_iterator it = body->children().begin();
         it != body->children().end();
         ++it)
      toXml(*it, retval);
  }

  void toXml(const PlexilUpdateBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(UPDATE_TAG, parent);
    toXml(body->update(), retval);
  }

  void toXml(const PlexilAssignmentBody* body, xml_node& parent)
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

  void toXml(const PlexilCommandBody* body, xml_node& parent)
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

  void toXml(const PlexilLibNodeCallBody* body, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval = appendElement(LIBRARYNODECALL_TAG, parent);
    appendNamedTextElement(NODEID_TAG, body->libNodeName().c_str(), retval);

    // format variable aliases
    for (PlexilAliasMap::const_iterator it = body->aliases().begin();
         it != body->aliases().end(); 
         ++it) {
      const std::pair<std::string, PlexilExpr *>& entry = *it;
      xml_node aliasXml = appendElement(ALIAS_TAG, retval);
      appendNamedTextElement(NODE_PARAMETER_TAG, entry.first.c_str(), aliasXml);
      toXml(entry.second, aliasXml);
    }

    // linked library node currently ignored
  }

  void toXml(const PlexilInternalVar* var, xml_node& parent)
    throw(ParserException) 
  {
    string name(NODE_TAG);
    if (dynamic_cast<PlexilOutcomeVar const *>(var))
      name.append("Outcome");
    else if (dynamic_cast<PlexilFailureVar const *>(var))
      name.append("Failure");
    else if (dynamic_cast<PlexilStateVar const *>(var))
      name.append("State");
    else if (dynamic_cast<PlexilCommandHandleVar const *>(var))
      name.append("CommandHandle");
    else if (dynamic_cast<PlexilTimepointVar const *>(var))
      name.append("Timepoint");
    name.append(VAR_TAG);
    xml_node retval = appendElement(name.c_str(), parent);
    toXml(var->ref(), retval);

    if (dynamic_cast<PlexilTimepointVar const *>(var)) {
      PlexilTimepointVar const * tp = (PlexilTimepointVar const *) var;
      appendNamedTextElement(STATEVAL_TAG, tp->state().c_str(), retval);
      appendNamedTextElement(TIMEPOINT_TAG, tp->timepoint().c_str(), retval);
    }
  }

  void toXml(PlexilState const *state, xml_node& parent)
    throw(ParserException) 
  {
    appendNamedTextElement(NAME_TAG, state->name().c_str(), parent);
    xml_node args = appendElement(ARGS_TAG, parent);
    for (std::vector<PlexilExpr *>::const_iterator it = state->args().begin();
         it != state->args().end();
         ++it)
      toXml(*it, args);
  }

  void toXml(PlexilUpdate const *update, xml_node& parent)
    throw(ParserException) 
  {
    for (std::vector<std::pair<string, PlexilExpr *> >::const_iterator it = update->pairs().begin(); 
         it != update->pairs().end();
         ++it) {
      xml_node pair = appendElement(PAIR_TAG, parent);
      appendNamedTextElement(NAME_TAG, it->first.c_str(), pair);
      toXml(it->second, pair);
    }
  }

  void toXml(PlexilNodeRef const *ref, xml_node& parent)
    throw(ParserException) 
  {
    xml_node retval;
    switch (ref->dir()) {
    case PlexilNodeRef::SELF:
      retval = appendElement(NODEREF_TAG, parent);
      retval.append_attribute(DIR_ATTR).set_value("self");
      break;

    case PlexilNodeRef::PARENT:
      retval = appendElement(NODEREF_TAG, parent);
      retval.append_attribute(DIR_ATTR).set_value("parent");
      break;

    case PlexilNodeRef::CHILD:
      retval = appendNamedTextElement(NODEREF_TAG, ref->name().c_str(), parent);
      retval.append_attribute(DIR_ATTR).set_value("child");
      break;

    case PlexilNodeRef::SIBLING:
      retval = appendNamedTextElement(NODEREF_TAG, ref->name().c_str(), parent);
      retval.append_attribute(DIR_ATTR).set_value("sibling");
      break;

      // Directions that don't have a corresponding NodeRef variant
    case PlexilNodeRef::GRANDPARENT:
    case PlexilNodeRef::UNCLE:
      retval = appendNamedTextElement(NODEID_TAG, ref->name().c_str(), parent);
      break;

    default:
      checkParserException(ALWAYS_FAIL, "Unknown direction " << ref->dir());
      break;
    }

    // Shouldn't happen, but...
    checkParserException(retval, "Internal error: retval is empty");

    addSourceLocators(retval, ref);
  }

} // namespace PLEXIL
