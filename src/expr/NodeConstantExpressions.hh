/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NODE_CONSTANT_EXPRESSIONS_HH
#define PLEXIL_NODE_CONSTANT_EXPRESSIONS_HH

#include "Constant.hh"

#include "CommandHandle.hh"
#include "NodeConstants.hh"

namespace PLEXIL
{

  class NodeStateConstant : public Constant<NodeState>
  {
  public:
    NodeStateConstant(NodeState value);
    virtual ~NodeStateConstant() = default;
    char const *exprName() const;

  private:
    // Disallow default, copy, assign
    NodeStateConstant() = delete;
    NodeStateConstant(NodeStateConstant const &) = delete;
    NodeStateConstant(NodeStateConstant &&) = delete;
    NodeStateConstant &operator=(NodeStateConstant const &) = delete;
    NodeStateConstant &operator=(NodeStateConstant &&) = delete;
  };

  extern Expression *INACTIVE_CONSTANT();
  extern Expression *WAITING_CONSTANT();
  extern Expression *EXECUTING_CONSTANT();
  extern Expression *ITERATION_ENDED_CONSTANT();
  extern Expression *FINISHED_CONSTANT();
  extern Expression *FAILING_CONSTANT();
  extern Expression *FINISHING_CONSTANT();
  
  class NodeOutcomeConstant : public Constant<NodeOutcome>
  {
  public:
    NodeOutcomeConstant(NodeOutcome value);
    virtual ~NodeOutcomeConstant() = default;
    char const *exprName() const;

  private:
    // Disallow default, copy, assign
    NodeOutcomeConstant() = delete;
    NodeOutcomeConstant(NodeOutcomeConstant const &) = delete;
    NodeOutcomeConstant(NodeOutcomeConstant &&) = delete;
    NodeOutcomeConstant &operator=(NodeOutcomeConstant const &) = delete;
    NodeOutcomeConstant &operator=(NodeOutcomeConstant &&) = delete;
  };

  extern Expression *SUCCESS_CONSTANT();
  extern Expression *FAILURE_CONSTANT();
  extern Expression *SKIPPED_CONSTANT();
  extern Expression *INTERRUPTED_CONSTANT();
  
  class FailureTypeConstant : public Constant<FailureType>
  {
  public:
    FailureTypeConstant(FailureType value);
    virtual ~FailureTypeConstant() = default;
    char const *exprName() const;

  private:
    // Disallow default, copy, assign
    FailureTypeConstant() = delete;
    FailureTypeConstant(FailureTypeConstant const &) = delete;
    FailureTypeConstant(FailureTypeConstant &&) = delete;
    FailureTypeConstant &operator=(FailureTypeConstant const &) = delete;
    FailureTypeConstant &operator=(FailureTypeConstant &&) = delete;
  };

  extern Expression *PRE_CONDITION_FAILED_CONSTANT();
  extern Expression *POST_CONDITION_FAILED_CONSTANT();
  extern Expression *INVARIANT_CONDITION_FAILED_CONSTANT();
  extern Expression *PARENT_FAILED_CONSTANT();
  extern Expression *EXITED_CONSTANT();
  extern Expression *PARENT_EXITED_CONSTANT();
  
  class CommandHandleConstant : public Constant<CommandHandleValue>
  {
  public:
    CommandHandleConstant(CommandHandleValue value);
    virtual ~CommandHandleConstant() = default;
    char const *exprName() const;

  private:
    // Disallow default, copy, assign
    CommandHandleConstant() = delete;
    CommandHandleConstant(CommandHandleConstant const &) = delete;
    CommandHandleConstant(CommandHandleConstant &&) = delete;
    CommandHandleConstant &operator=(CommandHandleConstant const &) = delete;
    CommandHandleConstant &operator=(CommandHandleConstant &&) = delete;
  };

  extern Expression *COMMAND_SENT_TO_SYSTEM_CONSTANT();
  extern Expression *COMMAND_ACCEPTED_CONSTANT();
  extern Expression *COMMAND_RCVD_BY_SYSTEM_CONSTANT();
  extern Expression *COMMAND_FAILED_CONSTANT();
  extern Expression *COMMAND_DENIED_CONSTANT();
  extern Expression *COMMAND_SUCCESS_CONSTANT();

} // namespace PLEXIL

#endif // PLEXIL_NODE_CONSTANT_EXPRESSIONS_HH
