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

#include "NodeVariables.hh"

#include "NodeImpl.hh"

#include <ostream>

namespace PLEXIL
{
  //
  // StateVariable
  //

  StateVariable::StateVariable(NodeImpl &node)
    : m_node(node)
  {
  }

  bool StateVariable::getValue(NodeState &result) const
  {
    result = m_node.getState();
    return true;
  }
      
  void StateVariable::printValue(std::ostream& s) const
  {
    s << nodeStateName(m_node.getState());
  }

  void StateVariable::printSpecialized(std::ostream& s) const
  {
    s << m_node.getNodeId() << ' ';
  }

  void StateVariable::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(&m_node);
  }

  // Don't listen to me, listen to the boss.
  void StateVariable::addListener(ExpressionListener *ptr)
  {
    m_node.addListener(ptr);
  }

  void StateVariable::removeListener(ExpressionListener *ptr)
  {
    m_node.removeListener(ptr);
  }

  //
  // OutcomeVariable
  //

  OutcomeVariable::OutcomeVariable(NodeImpl &node)
    : m_node(node)
  {
  }

  bool OutcomeVariable::isKnown() const
  {
    NodeOutcome outcome = m_node.getOutcome();
    return (outcome != NO_OUTCOME);
  }

  bool OutcomeVariable::getValue(NodeOutcome &result) const
  {
    NodeOutcome outcome = m_node.getOutcome();
    if (outcome == NO_OUTCOME)
      return false;
    result = outcome;
    return true;
  }
      
  void OutcomeVariable::printValue(std::ostream& s) const
  {
    NodeOutcome outcome = m_node.getOutcome();
    if (outcome == NO_OUTCOME)
      s << "[unknown_value]";
    else
      s << outcomeName((NodeOutcome) outcome);
  }

  void OutcomeVariable::printSpecialized(std::ostream& s) const
  {
    s << m_node.getNodeId() << ' ';
  }

  void OutcomeVariable::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(&m_node);
  }

  // Don't listen to me, listen to the boss.
  void OutcomeVariable::addListener(ExpressionListener *ptr)
  {
    m_node.addListener(ptr);
  }

  void OutcomeVariable::removeListener(ExpressionListener *ptr)
  {
    m_node.removeListener(ptr);
  }

  //
  // FailureVariable
  //

  FailureVariable::FailureVariable(NodeImpl &node)
    : m_node(node)
  {
  }

  bool FailureVariable::isKnown() const
  {
    FailureType ftype = m_node.getFailureType();
    return (ftype != NO_FAILURE);
  }

  bool FailureVariable::getValue(FailureType &result) const
  {
    FailureType ftype = m_node.getFailureType();
    if (ftype == NO_FAILURE)
      return false;
    result = ftype;
    return true;
  }
      
  void FailureVariable::printValue(std::ostream& s) const
  {
    s << failureTypeName(m_node.getFailureType());
  }

  void FailureVariable::printSpecialized(std::ostream& s) const
  {
    s << m_node.getNodeId() << ' ';
  }

  void FailureVariable::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(&m_node);
  }

  // Don't listen to me, listen to the boss.
  void FailureVariable::addListener(ExpressionListener *ptr)
  {
    m_node.addListener(ptr);
  }

  void FailureVariable::removeListener(ExpressionListener *ptr)
  {
    m_node.removeListener(ptr);
  }

} // namespace PLEXIL
