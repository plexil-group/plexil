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

#include "NodeOperatorImpl.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{

  // Default method for unspecialized types
  template <typename R>
  ValueType NodeOperatorImpl<R>::valueType() const
  {
    return PlexilValueType<R>::value;
  }

  // Convenience methods
  template <typename R>
  bool NodeOperatorImpl<R>::isKnown(NodeImpl const *node) const
  {
    R dummy;
    return (*this)(dummy, node);
  }

  template <typename R>
  void NodeOperatorImpl<R>::printValue(std::ostream &s, NodeImpl const *node) const
  {
    R temp;
    if ((*this)(temp, node))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename R>
  Value NodeOperatorImpl<R>::toValue(NodeImpl const *node) const
  {
    R temp;
    if ((*this)(temp, node))
      return Value(temp);
    else
      return Value(PlexilValueType<R>::value);
  }

  //
  // Explicit instantiations
  //

  template class NodeOperatorImpl<Boolean>;

} // namespace PLEXIL
