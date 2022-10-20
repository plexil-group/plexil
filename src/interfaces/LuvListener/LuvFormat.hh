/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#ifndef Luv_Format_hh
#define Luv_Format_hh

#include "Value.hh"
#include "pugixml.hpp"

#include <iosfwd>

namespace PLEXIL {

  // Forward references in PLEXIL namespace
  class Expression;
  class Node;
  struct NodeTransition;

  //
  // Format constants
  //

  // End-of-message marker
  static constexpr const char LUV_END_OF_MESSAGE = (char) 4;

  class LuvFormat {
  public:

    /**
     * @brief Construct the PlanInfo header XML.
     * @param s The stream to write the XML to.
     * @param block Whether the viewer should block.
     */
    static void formatPlanInfo(std::ostream &s, bool block);

    /**
     * @brief Construct the node state transition XML.
     * @param s The stream to write the XML to.
     * @param trans Const reference to the node state transition record.
     */
    static void formatTransition(std::ostream &s, 
                                 NodeTransition const &trans);

    /**
     * @brief Construct the assignment XML.
     * @param s The stream to write the XML to.
     * @param dest The expression being assigned to.
     * @param destName The variable name of the expression.
     * @param value The internal representation of the new value.
     */
    static void formatAssignment(std::ostream &s,
                                 Expression const *dest,
                                 std::string const &destName,
                                 Value const &value);

    /**
     * @brief Construct the message representing a new plan.
     * @param s The stream to write the XML to.
     * @param plan The XML DOM representation of the new plan.
     */
    static void formatPlan(std::ostream &s,
                           pugi::xml_node const plan);

    /**
     * @brief Construct the message representing a new library node.
     * @param s The stream to write the XML to.
     * @param libNode The intermediate representation of the library node.
     */
    static void formatLibrary(std::ostream& s,
                              pugi::xml_node const libNode);

  private:

    //
    // Deliberately unimplemented
    //

    // Constructors
    LuvFormat();
    LuvFormat(const LuvFormat&);

    // Destructor
    ~LuvFormat();

    // Assignment
    LuvFormat& operator=(const LuvFormat&);
  };

}

#endif // Luv_Format_hh
