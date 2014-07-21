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

#ifndef PLEXIL_TO_XML_HH
#define PLEXIL_TO_XML_HH

#include "ParserException.hh"
#include "PlexilPlan.hh"
#include "pugixml.hpp"

namespace PLEXIL
{

  /**
   * @brief Turn the node back into an XML document.
   * @param node The node.
   * @return Pointer to a pugi::xml_document representing the node.
   * @note Caller is responsible for disposing of the result.
   */
  pugi::xml_document* toXml(PlexilNode const *node)
    throw(ParserException);


  void toXml(PlexilNode const *node, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilInterface const *intf, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilVar const *var, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilExpr const *expr, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilNodeBody const *body, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilVarRef const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilOp const *op, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilArrayElement const *op, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilLookup const *ref, pugi::xml_node& parent)
    throw(ParserException);
  pugi::xml_node toXml(PlexilChangeLookup const *lookup, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilValue const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilListBody const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilUpdateBody const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilAssignmentBody const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilCommandBody const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilLibNodeCallBody const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilInternalVar const *ref, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilState const *state, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilUpdate const *update, pugi::xml_node& parent)
    throw(ParserException);
  void toXml(PlexilNodeRef const *ref, pugi::xml_node& parent)
    throw(ParserException);

} // namespace PLEXIL

#endif // PLEXIL_TO_XML_HH
