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

#ifndef PLEXIL_NODE_OPERATORS_HH
#define PLEXIL_NODE_OPERATORS_HH

#include "NodeOperatorImpl.hh"

namespace PLEXIL
{

  class NodeInactive : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeInactive();

  public:
    virtual ~NodeInactive();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeInactive);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeWaiting : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeWaiting();

  public:
    virtual ~NodeWaiting();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeWaiting);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeExecuting : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeExecuting();

  public:
    virtual ~NodeExecuting();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeExecuting);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeIterationEnded : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeIterationEnded();

  public:
    virtual ~NodeIterationEnded();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeIterationEnded);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeFinished : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeFinished();

  public:
    virtual ~NodeFinished();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeFinished);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeSucceeded : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeSucceeded();

  public:
    virtual ~NodeSucceeded();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeSucceeded);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeFailed : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeFailed();

  public:
    virtual ~NodeFailed();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeFailed);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeSkipped : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeSkipped();

  public:
    virtual ~NodeSkipped();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeSkipped);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodePostconditionFailed : public NodeOperatorImpl<Boolean>
  {
  private:
    NodePostconditionFailed();

  public:
    virtual ~NodePostconditionFailed();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodePostconditionFailed);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

  class NodeNoChildFailed : public NodeOperatorImpl<Boolean>
  {
  private:
    NodeNoChildFailed();

  public:
    virtual ~NodeNoChildFailed();
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeNoChildFailed);

    bool operator()(Boolean &result, NodeImpl const *node) const;
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const;
  };

}

#endif // PLEXIL_NODE_OPERATORS_HH
