/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "ExpressionImpl.hh"

#include "ArrayImpl.hh"
#include "Error.hh"

namespace PLEXIL
{

  /**
   * @brief Print the expression's value to the given stream.
   * @param s The output stream.
   * @note Default method, specializations should be implemented as appropriate.
   */
  template <typename T>
  void ExpressionImpl<T>::printValue(std::ostream &s) const
  {
    T temp;
    if (this->getValueImpl(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename T>
  void ExpressionImpl<ArrayImpl<T> >::printValue(std::ostream &s) const
  {
    ArrayImpl<T> const *temp;
    if (this->getValuePointerImpl(temp))
      PLEXIL::printValue(*temp, s);
    else
      s << "UNKNOWN";
  }

  template <>
  void ExpressionImpl<String>::printValue(std::ostream &s) const
  {
    String const *temp;
    if (this->getValuePointerImpl(temp))
      PLEXIL::printValue(*temp, s);
    else
      s << "UNKNOWN";
  }


  //
  // Explicit instantiations
  //
  template class ExpressionImpl<Boolean>;
  template class ExpressionImpl<NodeState>;
  template class ExpressionImpl<NodeOutcome>;
  template class ExpressionImpl<FailureType>;
  template class ExpressionImpl<CommandHandleValue>;
  template class ExpressionImpl<Integer>;
  template class ExpressionImpl<Real>;

  //template class ExpressionImpl<String>; // explicitly defined in header file

  template class ExpressionImpl<BooleanArray>;
  template class ExpressionImpl<IntegerArray>;
  template class ExpressionImpl<RealArray>;
  template class ExpressionImpl<StringArray>;

} // namespace PLEXIL
