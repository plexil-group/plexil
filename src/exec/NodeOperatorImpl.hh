// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_NODE_OPERATOR_IMPL_HH
#define PLEXIL_NODE_OPERATOR_IMPL_HH

#include "NodeOperator.hh"

#include "Value.hh"

namespace PLEXIL
{

  //! \class NodeOperatorImpl
  //! \brief Implements common core functionality for NodeOperator implementations.
  //! \ingroup Exec-Core
  template <typename R>
  class NodeOperatorImpl : public NodeOperator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~NodeOperatorImpl() = default;

    //
    // Default methods, based on R
    //

    //! \brief Return the value type of this Operation's result.
    //! \return The ValueType.
    virtual ValueType valueType() const override;

    //! \brief Is the result of this operation on this Node known?
    //! \param node Const pointer to the node.
    //! \return true if known, false if unknown.
    virtual bool isKnown(NodeImpl const *node) const override;

    //! \brief Print the result of this Operation on this Node to an output stream.
    //! \param s The stream.
    //! \param node Const pointer to the node.
    virtual void printValue(std::ostream &s, NodeImpl const *node) const override;

    //! \brief Return the result of this operation on this Node as a Value instance.
    //! \param node Const pointer to the Node.
    //! \return The Value.
    virtual Value toValue(NodeImpl const *node) const override;

  protected:

    // Base class shouldn't be instantiated by itself
    NodeOperatorImpl(std::string const &name)
      : NodeOperator(name)
    {
    }

  private:

    // Default, copy, move constructors, assignment operators unimplemented.
    NodeOperatorImpl() = delete;
    NodeOperatorImpl(NodeOperatorImpl const &) = delete;
    NodeOperatorImpl(NodeOperatorImpl &&) = delete;
    NodeOperatorImpl &operator=(NodeOperatorImpl const &) = delete;
    NodeOperatorImpl &operator=(NodeOperatorImpl &&) = delete;

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
