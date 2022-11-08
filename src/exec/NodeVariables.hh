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

#ifndef PLEXIL_NODE_VARIABLES_HH
#define PLEXIL_NODE_VARIABLES_HH

//
// Plan-visible expressions derived from node internal state
//

#include "GetValueImpl.hh"

namespace PLEXIL
{

  // Forward declarations
  class NodeImpl;

  //! \class StateVariable
  //! \brief Represents the state of a %PLEXIL Node.
  //! \ingroup Exec-Core
  class StateVariable final :
    public GetValueImpl<NodeState>
  {
  public:

    //! \brief Constructor.
    //! \param node Reference to the NodeImpl which owns the variable.
    StateVariable(NodeImpl &node);

    //! \brief Virtual destructor.
    virtual ~StateVariable() = default;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override
    {
      return "StateVariable";
    }

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    //! \note Always returns NODE_STATE_TYPE.
    virtual ValueType valueType() const override
    {
      return NODE_STATE_TYPE;
    }

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    //! \note A StateVariable's value is always known.
    virtual bool isKnown() const override
    {
      return true;
    }

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(NodeState &) const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream& s) const override;

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override;

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    virtual void addListener(ExpressionListener *l) override;

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    virtual void removeListener(ExpressionListener *l) override;

  protected:

    //
    // Expression internal API
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;

  private:

    // Default, copy, move constructors, assignment operators not implemented.
    StateVariable() = delete;
    StateVariable(StateVariable const &) = delete;
    StateVariable(StateVariable &&) = delete;
    StateVariable &operator=(StateVariable const &) = delete;
    StateVariable &operator=(StateVariable &&) = delete;

    NodeImpl &m_node; //!< The owning node.
  };

  //! \class OutcomeVariable
  //! \brief Represents the outcome of a %PLEXIL Node.
  //! \ingroup Exec-Core
  class OutcomeVariable final :
    public GetValueImpl<NodeOutcome>
  {
  public:

    //! \brief Constructor.
    //! \param node Reference to the NodeImpl which owns the variable.
    OutcomeVariable(NodeImpl &node);

    //! \brief Virtual destructor.
    virtual ~OutcomeVariable() = default;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override
    {
      return "OutcomeVariable";
    }

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    //! \note Always returns OUTCOME_TYPE.
    virtual ValueType valueType() const override
    {
      return OUTCOME_TYPE;
    }

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(NodeOutcome &) const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream& s) const override;

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override;

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    virtual void addListener(ExpressionListener *l) override;

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    virtual void removeListener(ExpressionListener *l) override;

  protected:

    //
    // Expression internal API
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;

  private:

    // Default, copy, move constructors, assignment operators not implemented.
    OutcomeVariable() = delete;
    OutcomeVariable(OutcomeVariable const &) = delete;
    OutcomeVariable(OutcomeVariable &&) = delete;
    OutcomeVariable &operator=(OutcomeVariable const &) = delete;
    OutcomeVariable &operator=(OutcomeVariable &&) = delete;

    NodeImpl &m_node; //!< The owning node.
  };

  //! \class FailureVariable
  //! \brief Represents the failure type of a %PLEXIL Node.
  //! \ingroup Exec-Core
  class FailureVariable final :
    public GetValueImpl<FailureType>
  {
  public:

    //! \brief Constructor.
    //! \param node Reference to the NodeImpl which owns the variable.
    FailureVariable(NodeImpl &node);

    //! \brief Virtual destructor.
    virtual ~FailureVariable() = default;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override
    {
      return "FailureVariable";
    }

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    //! \note Always returns NODE_STATE_TYPE.
    virtual ValueType valueType() const override
    {
      return FAILURE_TYPE;
    }

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(FailureType &) const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream& s) const override;

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override;

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    virtual void addListener(ExpressionListener *l) override;

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    virtual void removeListener(ExpressionListener *l) override;

  protected:

    //
    // Expression internal API
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;

  private:

    // Not implemented
    // Default, copy, move constructors, assignment operators not implemented.
    FailureVariable() = delete;
    FailureVariable(FailureVariable const &) = delete;
    FailureVariable(FailureVariable &&) = delete;
    FailureVariable &operator=(FailureVariable const &) = delete;
    FailureVariable &operator=(FailureVariable &&) = delete;

    NodeImpl &m_node; //!< The owning node.
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_VARIABLES_HH
