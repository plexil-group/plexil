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

#include "Expression.hh"

#include "Error.hh"
//#include "Node.hh"

#include <iostream>
#include <sstream>

namespace PLEXIL {

  Expression::Expression()
  {
  }

  Expression::~Expression()
  {
  }

  bool Expression::isAssignable() const
  {
    return false;
  }

  bool Expression::isConstant() const
  {
    return false;
  }

  // TODO: add exprName, typeName
  void Expression::print(std::ostream& s) const
  {
    s << "(" << getId()
	  << "[" << (this->isActive() ? "a" : "i")
	  << "](";
	this->printValue(s);
	s << "): ";
  }

  // Stream-style print operator
  std::ostream& operator<<(std::ostream& s, const Expression& e)
  {
    e.print(s);
    return s;
  }

  std::string Expression::toString() const
  {
    std::ostringstream s;
    this->print(s);
    return s.str();
  }

  std::string Expression::valueString() const
  {
    std::ostringstream s;
    this->printValue(s);
    return s.str();
  }

  void Expression::notifyChanged()
  {
  }

  bool Expression::getValue(double &) const
  {
    assertTrue_2(ALWAYS_FAIL, "Error: getValue() not implemented for Real for this expression");
    return false;
  }

  bool Expression::getValue(int32_t &) const
  {
    assertTrue_2(ALWAYS_FAIL, "Error: getValue() not implemented for Integer for this expression");
    return false;
  }

  bool Expression::getValue(uint16_t &) const
  {
    assertTrue_2(ALWAYS_FAIL, "Error: getValue() not implemented for internal types for this expression");
    return false;
  }

  bool Expression::getValue(bool &) const
  {
    assertTrue_2(ALWAYS_FAIL, "Error: getValue() not implemented for Boolean for this expression");
    return false;
  }

  bool Expression::getValue(std::string &) const
  {
    assertTrue_2(ALWAYS_FAIL, "Error: getValue() not implemented for String for this expression");
    return false;
  }

} // namespace PLEXIL