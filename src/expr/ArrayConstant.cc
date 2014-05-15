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

#include "ArrayConstant.hh"

namespace PLEXIL
{
  template <typename T>
  ArrayConstant<T>::ArrayConstant()
    : Constant<Array<T> >()
  {
  }

  template <typename T>
  ArrayConstant<T>::ArrayConstant(const ArrayConstant &other)
  : Constant<Array<T> >(other)
  {
  }

  template <typename T>
  ArrayConstant<T>::ArrayConstant(const Array<T> &value)
    : Constant<Array<T> >(value)
  {
  }

  template <typename T>
  ArrayConstant<T>::ArrayConstant(const std::vector<T> &value)
    : Constant<Array<T> >(Array<T>(value))
  {
  }

  template <typename T>
  ArrayConstant<T>::~ArrayConstant()
  {
  }

  //
  // Explicit implementations
  //

  template class ArrayConstant<bool>;
  template class ArrayConstant<int32_t>;
  template class ArrayConstant<double>;
  template class ArrayConstant<std::string>;

} // namespace PLEXIL
