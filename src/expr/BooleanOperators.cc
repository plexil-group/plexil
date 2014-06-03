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

#include "BooleanOperators.hh"
#include "Expression.hh"

namespace PLEXIL
{
  BooleanNot::BooleanNot()
    : Operator<bool>()
  {
    this->setName("NOT");
  }

  BooleanNot::~BooleanNot()
  {
  }

  bool BooleanNot::operator()(bool &result, const ExpressionId &arg) const
  {
    bool temp;
    if (!arg->getValue(temp))
      return false;
    result = !temp;
    return true;
  }

  BooleanOr::BooleanOr()
    : Operator<bool>()
  {
    this->setName("OR");
  }

  BooleanOr::~BooleanOr()
  {
  }

  bool BooleanOr::operator()(bool &result, const ExpressionId &arg) const
  {
    return arg->getValue(result);
  }

  bool BooleanOr::operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const
  {
    bool temp, known;
    if (argA->getValue(temp)) {
      if (temp) {
        result = true;
        return true;
      }
      // A known but false
      if ((known = argB->getValue(temp)))
        result = temp;
      return known;
    }
    // A unknown
    if (argB->getValue(temp) && temp) {
      result = temp;
      return true;
    }
    return false;
  }

  bool BooleanOr::operator()(bool &result, const std::vector<ExpressionId> &args) const
  {
    bool anyKnown = false;
    for (std::vector<ExpressionId>::const_iterator it = args.begin();
         it != args.end();
         ++it) {
      bool temp;
      if ((*it)->getValue(temp)) {
        // Return if any arg is known and true
        if (temp) {
          result = true;
          return true;
        }
        anyKnown = true; // but no true value yet
      }
    }
    if (anyKnown)
      result = false; // or we would have returned above
    return anyKnown;
  }

  //
  // AND
  //

  BooleanAnd::BooleanAnd()
    : Operator<bool>()
  {
    this->setName("AND");
  }

  BooleanAnd::~BooleanAnd()
  {
  }

  bool BooleanAnd::operator()(bool &result, const ExpressionId &arg) const
  {
    return arg->getValue(result);
  }

  bool BooleanAnd::operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const
  {
    bool temp, known;
    if (argA->getValue(temp)) {
      // A known
      if (!temp) {
        result = false; // cannot be true
        return true;
      }
      // A known and true
      if ((known = (argB->getValue(temp))))
        result = temp;
      return known;
    }
    // A unknown
    if (argB->getValue(temp) && !temp) {
      result = false; // cannot be true
      return true;
    }
    return false; // cannot be known
  }

  bool BooleanAnd::operator()(bool &result, const std::vector<ExpressionId> &args) const
  {
    bool allKnown = true;
    for (std::vector<ExpressionId>::const_iterator it = args.begin();
         it != args.end();
         ++it) {
      bool temp;
      if ((*it)->getValue(temp)) {
        if (!temp) {
          // Any known and false -> result known and false
          result = false;
          return true;
        }
      }
      else
        allKnown = false;
    }
    if (allKnown)
      result = true;
    return allKnown;
  }

  BooleanXor::BooleanXor()
    : Operator<bool>()
  {
    this->setName("XOR");
  }

  BooleanXor::~BooleanXor()
  {
  }

  bool BooleanXor::operator()(bool &result, const ExpressionId &arg) const
  {
    return arg->getValue(result);
  }

  bool BooleanXor::operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const
  {
    bool temp1, temp2;
    if (!argA->getValue(temp1))
      return false;
    if (!argB->getValue(temp2))
      return false;
    result = (temp1 != temp2);
    return true;
  }

  bool BooleanXor::operator()(bool &result, const std::vector<ExpressionId> &args) const
  {
    bool temp1 = false;
    for (std::vector<ExpressionId>::const_iterator it = args.begin();
         it != args.end();
         ++it) {
      bool temp2;
      // Return unknown if any arg is unknown
      if ((*it)->getValue(temp2))
        return false;
      temp1 = (temp1 != temp2);
    }
    // Can only get here if all known
    result = temp1;
    return true;
  }

} // namespace PLEXIL

