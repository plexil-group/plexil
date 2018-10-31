/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "Propagator.hh"

namespace PLEXIL {

  //
  // Propagator
  //

  Propagator::Propagator()
    : Notifier()
  {
  }

  Propagator::~Propagator()
  {
  }

  //
  // Expression listener graph construction and teardown
  // 

  //
  // In order to reduce memory usage and graph propagation delays, we try to
  // minimize the number of listeners added to expressions.
  //
  // There are three cases where we want to add a listener to an expression:
  //  1. Root expression, i.e. a node condition. This is the expression on which
  //     addListener() is explicitly called during plan loading.
  //  2. Interior subexpression whose value can change independently of its
  //     parameters (e.g. Lookup, random number generator).
  //  3. Leaf expression that can change, i.e. variable.
  //
  // We only add listeners to expressions that are propagation sources,
  // whether they are leaves or interior nodes of the tree.

  //
  // Internal helper for addListener() method
  //
  
  class AddListenerHelper : public ListenableUnaryOperator
  {
  public:
    AddListenerHelper(ExpressionListener *listener)
      : l(listener)
    {
    }

    virtual ~AddListenerHelper()
    {
    }

    virtual void operator()(Listenable *exp) const
    {
      if (exp->isPropagationSource())
        // This expression can independently generate notifications,
        // so add requested listener here
        exp->addListener(l);
      else 
        // Recurse through subexpressions
        exp->doSubexprs(*this);
    }

  private:
    ExpressionListener *l;
  };

  // Should only be called on expression root and internal nodes that are propagation sources.
  void Propagator::addListener(ExpressionListener *ptr)
  {
    if (!hasListeners())
      doSubexprs(AddListenerHelper(this));
    Notifier::addListener(ptr);
  }

  //
  // Internal helper for removeListener() method
  //

  class RemoveListenerHelper : public ListenableUnaryOperator
  {
  public:
    RemoveListenerHelper(ExpressionListener *listener)
      : l(listener)
    {
    }

    virtual ~RemoveListenerHelper()
    {
    }

    virtual void operator()(Listenable *exp) const
    {
      if (exp->isPropagationSource())
        exp->removeListener(l);
      else
        exp->doSubexprs(*this);
    }

  private:
    ExpressionListener *l;
  };

  void Propagator::removeListener(ExpressionListener *ptr)
  {
    Notifier::removeListener(ptr);
    if (!hasListeners())
      doSubexprs(RemoveListenerHelper(this));
  }

  void Propagator::notifyChanged()
  {
    if (this->isActive())
      this->handleChange();
  }

  void Propagator::handleChange()
  {
    publishChange();
  }

} // namespace PLEXIL
