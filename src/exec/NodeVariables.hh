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

#ifndef PLEXIL_NODE_VARIABLES_HH
#define PLEXIL_NODE_VARIABLES_HH

//
// Plan-visible expressions derived from node internal state
//

#include "GetValueImpl.hh"
#include "NodeConstants.hh"

namespace PLEXIL {

  // Forward references
  class NodeImpl;

  class StateVariable final :
    public GetValueImpl<NodeState>
  {
  public:
    /**
     * @brief Constructor.
     */
    StateVariable(NodeImpl &node);

    /**
     * @brief Destructor.
     */
    ~StateVariable();

    const char *exprName() const
    {
      return "StateVariable";
    }

    /**
     * @brief Get the type of the expression's value.
     */
    ValueType valueType() const
    {
      return NODE_STATE_TYPE;
    }

    inline bool isKnown() const
    {
      return true;
    }

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValue(NodeState &) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

    // Listenable API
    virtual void doSubexprs(ListenableUnaryOperator const &oper);

    // Listenable API
    void addListener(ExpressionListener *l);
    void removeListener(ExpressionListener *l);

  protected:

    //
    // Expression internal API
    //

    void printSpecialized(std::ostream &s) const;

  private:
    // Not implemented
    StateVariable() = delete;
    StateVariable(const StateVariable &) = delete;
    StateVariable(StateVariable &&) = delete;
    StateVariable &operator=(const StateVariable &) = delete;
    StateVariable &operator=(StateVariable &&) = delete;

    NodeImpl &m_node;
  };

  class OutcomeVariable final :
    public GetValueImpl<NodeOutcome>
  {
  public:
    /**
     * @brief Constructor.
     */
    OutcomeVariable(NodeImpl &node);

    /**
     * @brief Destructor.
     */
    ~OutcomeVariable();

    const char *exprName() const
    {
      return "OutcomeVariable";
    }

    /**
     * @brief Get the type of the expression's value.
     */
    ValueType valueType() const
    {
      return OUTCOME_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValue(NodeOutcome &) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

    // Listenable API
    virtual void doSubexprs(ListenableUnaryOperator const &oper);

    // Listenable API
    void addListener(ExpressionListener *l);
    void removeListener(ExpressionListener *l);

  protected:

    //
    // Expression internal API
    //

    void printSpecialized(std::ostream &s) const;

  private:

    // Not implemented
    OutcomeVariable() = delete;
    OutcomeVariable(const OutcomeVariable &) = delete;
    OutcomeVariable(OutcomeVariable &&) = delete;
    OutcomeVariable &operator=(const OutcomeVariable &) = delete;
    OutcomeVariable &operator=(OutcomeVariable &&) = delete;

    NodeImpl &m_node;
  };

  class FailureVariable final :
    public GetValueImpl<FailureType>
  {
  public:
    /**
     * @brief Constructor.
     */
    FailureVariable(NodeImpl &node);

    /**
     * @brief Destructor.
     */
    ~FailureVariable();

    const char *exprName() const
    {
      return "FailureVariable";
    }

    /**
     * @brief Get the type of the expression's value.
     */
    ValueType valueType() const
    {
      return FAILURE_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValue(FailureType &) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

    // Listenable API
    virtual void doSubexprs(ListenableUnaryOperator const &oper);

    // Listenable API
    void addListener(ExpressionListener *l);
    void removeListener(ExpressionListener *l);

  protected:

    //
    // Expression internal API
    //

    void printSpecialized(std::ostream &s) const;

  private:

    // Not implemented
    FailureVariable() = delete;
    FailureVariable(const FailureVariable &) = delete;
    FailureVariable(FailureVariable &&) = delete;
    FailureVariable &operator=(const FailureVariable &) = delete;
    FailureVariable &operator=(FailureVariable &&) = delete;

    NodeImpl &m_node;
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_VARIABLES_HH
