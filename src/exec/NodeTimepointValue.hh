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

#ifndef PLEXIL_NODE_TIMEPOINT_VALUE_HH
#define PLEXIL_NODE_TIMEPOINT_VALUE_HH

#include "GetValueImpl.hh"
#include "Notifier.hh"

#include <memory> // std::unique_ptr

namespace PLEXIL
{

  // Forward declarations
  class NodeConnector;
  class NodeTimepointValue;

  // Type aliases
  using NodeTimepointValuePtr = std::unique_ptr<NodeTimepointValue>;

  //! \class NodeTimepointValue
  //! \brief An Expression representing the time of a node state transition.
  //! \ingroup Exec-Core
  class NodeTimepointValue final :
    public GetValueImpl<Real>,
    public Notifier
  {
  public:

    //! \brief Primary constructor.
    //! \param node Pointer to the Node (as a NodeConnector).
    //! \param state The desired node state.
    //! \param isEnd If false, refers to the state start time;
    //!        if true, the state end time.
    NodeTimepointValue(NodeConnector *node,
                       NodeState state,
                       bool isEnd);

    //! \brief Virtual destructor.
    ~NodeTimepointValue() = default;

    //
    // Listenable API
    //

    //! \brief Can this object generate change notifications?
    //! \return true if the object can change value of its own accord, false if not.
    virtual bool isPropagationSource() const override;

    //
    // Expression API
    //
    
    //! \brief Return the name of this expression.
    //! \return Pointer to const character string.
    virtual char const *getName() const override;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual char const *exprName() const override;

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const override;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(Real &result) const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const override;

    //
    // Local methods
    //

    //! \brief Get the state of this timepoint value.
    //! \return The NodeState.
    NodeState state() const;

    //! \brief Does this represent the start or end of a node state?
    //! \return false if start, true if end.
    bool isEnd() const;

    //! \brief Set the value of this timepoint.
    //! \param newval The new value.
    void setValue(Real newval);

    //! \brief Reset the value of this timepoint to unknown.
    void reset(); 

    //! \brief Get the pointer to the next object in the list.
    //! \return The pointer. May be NULL.
    NodeTimepointValue *next() const;

    //! \brief Set the pointer to the next object in the list.
    //! \param nxt The new pointer.
    void setNext(NodeTimepointValue *);

  protected:

    //
    // Expression internal API
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;


  private:

    // Default, copy, move constructors, assignment operators not implemented
    NodeTimepointValue() = delete;
    NodeTimepointValue(NodeTimepointValue const &) = delete;
    NodeTimepointValue(NodeTimepointValue &&) = delete;
    NodeTimepointValue &operator=(NodeTimepointValue const &) = delete;
    NodeTimepointValue &operator=(NodeTimepointValue &&) = delete;

    Real m_time;                   //!< The time of this event.
    NodeTimepointValuePtr m_next;  //!< Pointer to the next timepoint value.
    NodeConnector *m_node;         //!< Pointer to the owning node.
    NodeState const m_state;       //!< The state.  Only set at constructor time.
    bool const m_end;              //!< True if timepoint is state end, false if start.  Only set at constructor time.
    bool m_known;                  //!< True if known and assigned, false if not.
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_TIMEPOINT_VALUE_HH
