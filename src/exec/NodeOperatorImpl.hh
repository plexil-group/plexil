/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NODE_OPERATOR_IMPL_HH
#define PLEXIL_NODE_OPERATOR_IMPL_HH

#include "NodeOperator.hh"

#include "Value.hh"

namespace PLEXIL
{
  template <typename R>
  class NodeOperatorImpl : public NodeOperator
  {
  public:
    virtual ~NodeOperatorImpl()
    {
    }

    // Default methods, based on R
    ValueType valueType() const;

    // Not needed for Boolean, Integer, Real, internal values
    void *allocateCache() const
    {
      return NULL;
    }

    void deleteCache(void *ptr) const
    {
    }

    bool isKnown(NodeImpl const *node) const;
    void printValue(std::ostream &s, NodeImpl const *node) const;
    Value toValue(NodeImpl const *node) const;

  protected:

    // Base class shouldn't be instantiated by itself
    NodeOperatorImpl(std::string const &name)
      : NodeOperator(name)
    {
    }

  private:

    // Unimplemented
    NodeOperatorImpl();
    NodeOperatorImpl(NodeOperatorImpl const &);
    NodeOperatorImpl &operator=(NodeOperatorImpl const &);

  };

} // namespace PLEXIL

/**
 * @brief Helper macro, intended to implement "boilerplate" singleton accessors
 *        for classes derived from NodeOperatorImpl<R>.
 */
#define DECLARE_NODE_OPERATOR_STATIC_INSTANCE(CLASS) \
  static PLEXIL::NodeOperator const *instance() \
  { \
    static CLASS const sl_instance; \
    return static_cast<PLEXIL::NodeOperator const *>(&sl_instance); \
  }

#endif // PLEXIL_NODE_OPERATOR_IMPL_HH
