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

#include "LuvFormat.hh"

#include "Assignable.hh"
#include "Error.hh"
#include "Node.hh"

#include <iostream>

namespace PLEXIL {

  //
  // Local utilities
  //

  inline void simpleStartTag(std::ostream& s, const char* val) {
    s << '<' << val << ">";
  }

  inline void openStartTag(std::ostream& s, const char* val) {
    s << '<' << val << ' ';
  }

  inline void closeTag(std::ostream& s) {
    s << ">";
  }

  inline void attribute(std::ostream& s, const char* name, const char* val) {
    s << name << "=\"" << val << "\" ";
  }

  inline void endTag(std::ostream& s, const char* val) {
    s << "</" << val << "> ";
  }

  inline void emptyElement(std::ostream& s, const char* val) {
    s << '<' << val << " /> ";
  }

  /**
   * @brief Generate a simple XML element containing some text.
   * @param s The stream to write the element to.
   * @param tag The tag for the element.
   * @param text The text for the element.
   */
  void simpleTextElement(std::ostream& s,
                         const char* tag,
                         const char* text) {
    simpleStartTag(s, tag);
    s << text;
    endTag(s, tag);
  }

  //* Internal function for formatNodePath
  void formatNodePathInternal(std::ostream& s, 
                              Node const *node) {
    // Fill in parents recursively
    if (node->getParent())
      formatNodePathInternal(s, node->getParent());
    // Put ours at the end
    simpleTextElement(s, LuvFormat::NODE_ID_TAG(), node->getNodeId().c_str());
  }

  /**
   * @brief Generate the XML representation of the path to the node.
   * @param s The stream to write the XML to.
   * @param node The plan node whose path is being constructed.
   */
  void formatNodePath(std::ostream& s, 
                      Node const *node) {
    simpleStartTag(s, LuvFormat::NODE_PATH_TAG());
    formatNodePathInternal(s, node);
    endTag(s, LuvFormat::NODE_PATH_TAG());
  }

  /**
   * @brief Generate the XML representation of the current values of the node's conditions.
   * @param s The stream to write the XML to.
   * @param node The node whose conditions are being extracted.
   */
  void formatConditions(std::ostream& s, 
                        pugi::xml_node const plan)
  {
    plan.print(s, "", pugi::format_raw);
  }

  /**
   * @brief Construct the message representing a new library node.
   * @param s The stream to write the XML to.
   * @param plan The intermediate representation of the library node.
   */
  void LuvFormat::formatLibrary(std::ostream& s, 
                                pugi::xml_node const libNode)
  {
    // create a PLEXIL Library wrapper and stick the library node in it
    simpleStartTag(s, PLEXIL_LIBRARY_TAG());
    libNode.print(s, "", pugi::format_raw);
    endTag(s, PLEXIL_LIBRARY_TAG());
  }

}
