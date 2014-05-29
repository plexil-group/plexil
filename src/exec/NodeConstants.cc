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

namespace PLEXIL
{

  /**
   * @brief Table of state names.
   * @note Must be in same order as NodeState enum.
   */
  char const *ALL_STATE_NAMES[] =
    {"INACTIVE",
     "WAITING",
     "EXECUTING",
     "ITERATION_ENDED",
     "FINISHED",
     "FAILING",
     "FINISHING",
     "NO_STATE"
    };

  /**
   * @brief Table of outcome names.
   * @note Must be in same order as NodeOutcome enum.
   */
  char const *ALL_OUTCOME_NAMES[] =
    {"NO_OUTCOME",
     "SUCCESS",
     "FAILURE",
     "SKIPPED",
     "INTERRUPTED"
    };

  /**
   * @brief Table of failure type names.
   * @note Must be in same order as FailureType enum.
   */
  char const *ALL_FAILURE_NAMES[] =
    {"NO_FAILURE",
     "PRE_CONDITION_FAILED",
     "POST_CONDITION_FAILED",
     "INVARIANT_CONDITION_FAILED",
     "PARENT_FAILED",
     "EXITED",
     "PARENT_EXITED"
    };

  /**
   * @brief Table of command handle value names.
   * @note Must be in same order as CommandHandleValue enum above.
   */
  char const *ALL_COMMAND_HANDLE_NAMES[] =
    {"NO_COMMAND_HANDLE",
     "COMMAND_SENT_TO_SYSTEM",
     "COMMAND_ACCEPTED",
     "COMMAND_RCVD_BY_SYSTEM",
     "COMMAND_FAILED",
     "COMMAND_DENIED",
     "COMMAND_SUCCESS"
    };

  char const *nodeStateName(NodeState s)
  {
    if (s >= NODE_STATE_MAX)
      return ALL_STATE_NAMES[NO_NODE_STATE];
    return ALL_STATE_NAMES[s];
  }

  char const *outcomeName(NodeOutcome o)
  {
    if (o <= NO_OUTCOME || o >= OUTCOME_MAX)
      return ALL_OUTCOME_NAMES[0];
    return ALL_OUTCOME_NAMES[o - NO_OUTCOME];
  }

  char const *failureTypeName(FailureType f)
  {
    if (f <= NO_FAILURE || f >= FAILURE_TYPE_MAX)
      return ALL_FAILURE_NAMES[0];
    return ALL_FAILURE_NAMES[f - NO_FAILURE];
  }

  char const *commandHandleValueName(CommandHandleValue c)
  {
    if (c <= NO_COMMAND_HANDLE || c >= COMMAND_HANDLE_MAX)
      return ALL_COMMAND_HANDLE_NAMES[0];
    return ALL_COMMAND_HANDLE_NAMES[c - NO_COMMAND_HANDLE];
  }

} // namespace PLEXIL
