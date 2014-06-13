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

#include "StringOperators.hh"

namespace PLEXIL
{
  //
  // StringConcat
  //
  StringConcat::StringConcat()
    : Operator<std::string>()
  {
    this->setName("concat");
  }
  
  StringConcat::~StringConcat()
  {
  }

  bool StringConcat::checkArgCount(size_t /* count */) const
  {
    return true;
  }

  bool StringConcat::operator()(std::string &result,
                                const ExpressionId &arg) const
  {
    return arg->getValue(result);
  }

  bool StringConcat::operator()(std::string &result,
                                const ExpressionId &argA,
                                const ExpressionId &argB) const
  {
    std::string const *stringA, *stringB;
    if (!argA->getValuePointer(stringA)
        || !argB->getValuePointer(stringB))
      return false;
    result.reserve(stringA->size() + stringB->size());
    result = *stringA;
    result.append(*stringB);
    return true;
  }

  bool StringConcat::operator()(std::string &result, 
                                const std::vector<ExpressionId> &args) const
  {
    std::vector<std::string const *> vals(args.size());
    for (size_t i = 0; i < args.size(); ++i) {
      if (!args[i]->getValuePointer(vals[i]))
        return false;
    }
    size_t len = 0;
    for (size_t i = 0; i < args.size(); ++i)
      len += vals[i]->size();
    result.clear();
    result.reserve(len);
    for (size_t i = 0; i < args.size(); ++i)
      result.append(*(vals[i]));
    return true;
  }

  //
  // StringLength
  //
  StringLength::StringLength()
    : Operator<int32_t>()
  {
    this->setName("strlen");
  }

  StringLength::~StringLength()
  {
  }

  bool StringLength::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool StringLength::operator()(int32_t &result, const ExpressionId &arg) const
  {
    std::string const *str;
    if (!arg->getValuePointer(str))
      return false;
    result = str->size();
    return true;
  }

} // namespace PLEXIL
