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

#include "NodeOperators.hh"

#include "NodeImpl.hh"

namespace PLEXIL
{

  NodeInactive::NodeInactive()
    : NodeOperatorImpl<Boolean>("Inactive")
  {
  }

  bool NodeInactive::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == INACTIVE_STATE;
    return true;
  }

  NodeWaiting::NodeWaiting()
    : NodeOperatorImpl<Boolean>("Waiting")
  {
  }

  bool NodeWaiting::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == WAITING_STATE;
    return true;
  }

  NodeExecuting::NodeExecuting()
    : NodeOperatorImpl<Boolean>("Executing")
  {
  }

  bool NodeExecuting::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == EXECUTING_STATE;
    return true;
  }

  NodeIterationEnded::NodeIterationEnded()
    : NodeOperatorImpl<Boolean>("IterationEnded")
  {
  }

  bool NodeIterationEnded::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == ITERATION_ENDED_STATE;
    return true;
  }

  NodeFinished::NodeFinished()
    : NodeOperatorImpl<Boolean>("Finished")
  {
  }

  bool NodeFinished::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == FINISHED_STATE;
    return true;
  }

  NodeSucceeded::NodeSucceeded()
    : NodeOperatorImpl<Boolean>("Succeeded")
  {
  }

  bool NodeSucceeded::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == FINISHED_STATE
      && node->getOutcome() == SUCCESS_OUTCOME;
    return true;
  }

  NodeFailed::NodeFailed()
    : NodeOperatorImpl<Boolean>("Failed")
  {
  }

  bool NodeFailed::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getState() == FINISHED_STATE
      && node->getOutcome() == FAILURE_OUTCOME;
    return true;
  }

  NodeSkipped::NodeSkipped()
    : NodeOperatorImpl<Boolean>("Skipped")
  {
  }

  bool NodeSkipped::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getOutcome() == SKIPPED_OUTCOME;
    return true;
  }

  NodePostconditionFailed::NodePostconditionFailed()
    : NodeOperatorImpl<Boolean>("PostconditionFailed")
  {
  }

  bool NodePostconditionFailed::operator()(Boolean &result, NodeImpl const *node) const
  {
    result = node->getFailureType() == POST_CONDITION_FAILED;
    return true;
  }

  NodeNoChildFailed::NodeNoChildFailed()
    : NodeOperatorImpl<Boolean>("NoChildFailed")
  {
  }

  bool NodeNoChildFailed::operator()(Boolean &result, NodeImpl const *node) const
  {
    for (NodeImplPtr const &kid : node->getChildren()) {
      if (kid->getState() == FINISHED_STATE
          && kid->getOutcome() == FAILURE_OUTCOME) {
        result = false;
        return true;
      }
    }
    result = true;
    return true;
  }

  void NodeNoChildFailed::doPropagationSources(NodeImpl *node,
                                               ListenableUnaryOperator const &oper) const
  {
    for (NodeImplPtr &kid : node->getChildren())
      (oper)(kid.get());
  }

}
