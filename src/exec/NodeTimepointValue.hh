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

#ifndef PLEXIL_NODE_TIMEPOINT_VALUE_HH
#define PLEXIL_NODE_TIMEPOINT_VALUE_HH

#include "ExpressionImpl.hh"
#include "NodeConstants.hh"
#include "NotifierImpl.hh"

namespace PLEXIL
{
  class Node;

  class NodeTimepointValue : public NotifierImpl,
                             public ExpressionImpl<double> // FIXME
  {
  public:
    NodeTimepointValue(Node *node,
                       NodeState state,
                       bool isEnd);
    ~NodeTimepointValue();

    char const *getName() const;
    char const *exprName() const;
    ValueType const valueType() const;
    bool isKnown() const;

    bool getValueImpl(double &result) const; // FIXME

    void print(std::ostream &s) const;
    void printValue(std::ostream &s) const;

  protected:
    // Default method is adequate for now
    // void handleChange(Expression const *src);

  private:
    // not implemented
    NodeTimepointValue();
    NodeTimepointValue(NodeTimepointValue const &);
    NodeTimepointValue &operator=(NodeTimepointValue const &);

    Node *m_node;
    NodeState m_state;
    bool m_end;
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_TIMEPOINT_VALUE_HH
