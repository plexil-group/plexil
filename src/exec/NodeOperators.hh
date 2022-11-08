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

#ifndef PLEXIL_NODE_OPERATORS_HH
#define PLEXIL_NODE_OPERATORS_HH

#include "NodeOperatorImpl.hh"

namespace PLEXIL
{

  //! \class NodeInactive
  //! \brief A NodeOperator which returns true when the Node is in INACTIVE state,
  //!        false otherwise.
  //! \ingroup Exec-Core
  class NodeInactive : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeInactive();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeInactive() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeInactive);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeWaiting
  //! \brief A NodeOperator which returns true when the Node is in WAITING state,
  //!        false otherwise.
  //! \ingroup Exec-Core
  class NodeWaiting : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeWaiting();

  public:

    //! \brief Virtual destructor.
    virtual ~NodeWaiting() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeWaiting);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeExecuting
  //! \brief A NodeOperator which returns true when the Node is in EXECUTING state,
  //!        false otherwise.
  //! \ingroup Exec-Core
  class NodeExecuting : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeExecuting();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeExecuting() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeExecuting);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeIterationEnded
  //! \brief A NodeOperator which returns true when the Node is in ITERATION_ENDED state,
  //!        false otherwise.
  //! \ingroup Exec-Core
  class NodeIterationEnded : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeIterationEnded();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeIterationEnded() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeIterationEnded);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeFinished
  //! \brief A NodeOperator which returns true when the Node is in FINISHED state,
  //!        false otherwise.
  //! \ingroup Exec-Core
  class NodeFinished : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeFinished();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeFinished() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeFinished);

    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeSucceeded
  //! \brief A NodeOperator which returns true when the Node is in
  //!        FINISHED state and has an outcome of SUCCESS, false
  //!        otherwise.
  //! \ingroup Exec-Core
  class NodeSucceeded : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeSucceeded();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeSucceeded() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeSucceeded);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeFailed
  //! \brief A NodeOperator which returns true when the Node is in
  //!        FINISHED state and has an outcome of FAILURE, false
  //!        otherwise.
  //! \ingroup Exec-Core
  class NodeFailed : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeFailed();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeFailed() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeFailed);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeSkipped
  //! \brief A NodeOperator which returns true when the Node has an
  //!        outcome of SKIPPED, false otherwise.
  //! \ingroup Exec-Core
  class NodeSkipped : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeSkipped();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeSkipped() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeSkipped);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodePostconditionFailed
  //! \brief A NodeOperator which returns true when the Node has an
  //!        failure type of POST_CONDITION_FAILED, false otherwise.
  //! \ingroup Exec-Core
  class NodePostconditionFailed : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodePostconditionFailed();

  public:

    //! \brief Virtual destructor.
    virtual ~NodePostconditionFailed() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodePostconditionFailed);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;
  };

  //! \class NodeNoChildFailed
  //! \brief A NodeOperator which returns false when one or more of the node's children
  //!        are in FINISHED state and have an outcome of FAILURE, true otherwise.
  //! \ingroup Exec-Core
  class NodeNoChildFailed : public NodeOperatorImpl<Boolean>
  {
  private:

    //! \brief Default constructor.  Only accessible to singleton accessor.
    NodeNoChildFailed();

  public:

    //! \brief Virtual destuctor.
    virtual ~NodeNoChildFailed() = default;
    
    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(NodeNoChildFailed);

    //! \brief Execute the operation on the Node, and store the result.
    //! \param result Reference to the result variable.
    //! \param node Const pointer to the Node.
    //! \return true if the result is known, false if unknown.
    virtual bool operator()(Boolean &result, NodeImpl const *node) const override;

    //! \brief Map the given function over all propagation sources for this operator and Node.
    //! \param node Pointer to the Node.
    //! \param oper Const reference to the Operator.
    virtual void doPropagationSources(NodeImpl *node,
                                      ListenableUnaryOperator const &oper) const override;
  };

}

#endif // PLEXIL_NODE_OPERATORS_HH
