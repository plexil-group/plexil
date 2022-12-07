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

#ifndef PLEXIL_NODE_FUNCTION_HH
#define PLEXIL_NODE_FUNCTION_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{

  // Forward declarations
  class NodeImpl;
  class NodeOperator;

  //! \class NodeFunction
  //! \brief An abstract base class for functions whose value depends on
  //!        some property of a node or its neighbors.
  //! \ingroup Exec-Core
  class NodeFunction final
    : public Expression,
      public Propagator
  {
  public:

    //! \brief Constructor.
    //! \param op Pointer to the NodeOperator.
    //! \param exprs Pointer to the node on which this expression depends.
    NodeFunction(NodeOperator const *op, NodeImpl *exprs);

    //! \brief Virtual destructor.
    virtual ~NodeFunction() = default;

    //
    // Expression API
    //

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual char const *exprName() const override;

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const override;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The value.
    virtual Value toValue() const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const override;

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note Only Boolean-valued NodeOperators have been implemented to date.
    virtual bool getValue(Boolean &result) const override;

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override;

  protected:

    NodeOperator const *m_op; //!< Pointer to the NodeOperator instance.

  private:

    // Default, copy, move constructors, assignment operators not implemented.
    NodeFunction() = delete;
    NodeFunction(NodeFunction const &) = delete;
    NodeFunction(NodeFunction &&) = delete;
    NodeFunction &operator=(NodeFunction const &) = delete;
    NodeFunction &operator=(NodeFunction &&) = delete;

    NodeImpl *m_node;  //!< Pointer to the owning Node.
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_FUNCTION_HH
