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

#ifndef PLEXIL_NODE_VARIABLES_HH
#define PLEXIL_NODE_VARIABLES_HH

//
// Plan-visible expressions derived from node internal state
//

#include "ExpressionImpl.hh"
#include "NotifierImpl.hh"

namespace PLEXIL {

  class StateVariable : public NotifierImpl, public ExpressionImpl<uint16_t>
  {
  public:
    /**
     * @brief Constructor.
     */
    StateVariable(const NodeId& node);

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
    const ValueType valueType() const
    {
      return NODE_STATE_TYPE;
    }

    inline bool isKnown() const
    {
      return m_node.isId();
    }

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValueImpl(uint16_t &) const;
    bool getValuePointerImpl(uint16_t const *&) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

    // Notifier API
    inline bool isActive() const
    {
      return m_node.isId();
    }

    void activate() {}
    void deactivate() {}

  private:
    // Not implemented
    StateVariable();
    StateVariable(const StateVariable &);
    StateVariable &operator=(const StateVariable &);

    NodeId m_node;
  };

  class OutcomeVariable : public NotifierImpl, public ExpressionImpl<uint16_t>
  {
  public:
    /**
     * @brief Constructor.
     */
    OutcomeVariable(const NodeId& node);

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
    const ValueType valueType() const
    {
      return OUTCOME_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValueImpl(uint16_t &) const;
    bool getValuePointerImpl(uint16_t const *&) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

  private:

    // Not implemented
    OutcomeVariable();
    OutcomeVariable(const OutcomeVariable &);
    OutcomeVariable &operator=(const OutcomeVariable &);

    NodeId m_node;
  };

  class FailureVariable : public NotifierImpl, public ExpressionImpl<uint16_t>
  {
  public:
    /**
     * @brief Constructor.
     */
    FailureVariable(const NodeId& node);

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
    const ValueType valueType() const
    {
      return FAILURE_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValueImpl(uint16_t &) const;
    bool getValuePointerImpl(uint16_t const *&) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

  private:

    // Not implemented
    FailureVariable();
    FailureVariable(const FailureVariable &);
    FailureVariable &operator=(const FailureVariable &);

    NodeId m_node;
  };

  class CommandHandleVariable : public NotifierImpl, public ExpressionImpl<uint16_t>
  {
  public:
    /**
     * @brief Constructor.
     */
    CommandHandleVariable(const CommandId& cmd);

    /**
     * @brief Destructor.
     */
    ~CommandHandleVariable();

    const char *exprName() const
    {
      return "CommandHandleVariable";
    }

    /**
     * @brief Get the type of the expression's value.
     */
    const ValueType valueType() const
    {
      return COMMAND_HANDLE_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    bool getValueImpl(uint16_t &) const;
    bool getValuePointerImpl(uint16_t const *&) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

    void setName(const std::string &);

    void reset();

  private:

    // Not implemented
    CommandHandleVariable();
    CommandHandleVariable(const CommandHandleVariable &);
    CommandHandleVariable &operator=(const CommandHandleVariable &);

    CommandId m_command;
  };

  class TimepointVariable : public NotifierImpl,
                            public ExpressionImpl<double> // *** FIXME ***
  {
  public:
    /**
     * @brief Constructor.
     */
    TimepointVariable(const NodeId& node, NodeState state, bool isEnd);

    /**
     * @brief Destructor.
     */
    ~TimepointVariable();

    const char *exprName() const
    {
      return "TimepointVariable";
    }

    /**
     * @brief Get the type of the expression's value.
     */
    const ValueType valueType() const
    {
      return DATE_TYPE;
    }

    bool isKnown() const;

    /**
     * @brief Get the current value of the node's state.
     */
    // *** FIXME ***
    bool getValueImpl(double &) const;
    bool getValuePointerImpl(double const *&) const;

    /**
     * @brief Print the expression's value to the given stream.
     * @param s The output stream.
     */
    void printValue(std::ostream& s) const;

  private:

    // Not implemented
    TimepointVariable();
    TimepointVariable(const TimepointVariable &);
    TimepointVariable &operator=(const TimepointVariable &);

    NodeId m_node;
    const NodeState m_state;
    const bool m_isEnd, m_known;
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_VARIABLES_HH