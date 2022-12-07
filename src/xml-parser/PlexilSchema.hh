/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_SCHEMA_HH
#define PLEXIL_SCHEMA_HH

namespace PLEXIL
{
  constexpr char const PLEXIL_PLAN_TAG[] = "PlexilPlan";
  constexpr char const FILE_NAME_ATTR[] = "FileName";
  constexpr char const LINE_NO_ATTR[] = "LineNo";
  constexpr char const COL_NO_ATTR[] = "ColNo";

  constexpr char const GLOBAL_DECLARATIONS_TAG[] = "GlobalDeclarations";
  constexpr char const COMMAND_DECLARATION_TAG[] = "CommandDeclaration";
  constexpr char const RETURN_TAG[] = "Return";
  constexpr char const PARAMETER_TAG[] = "Parameter";
  constexpr char const ANY_PARAMETERS_TAG[] = "AnyParameters";

  // ???
  constexpr char const STATE_DECLARATION_TAG[] = "StateDeclaration";
  constexpr char const LIBRARY_NODE_DECLARATION_TAG[] = "LibraryNodeDeclaration";

  constexpr char const NODE_TAG[] = "Node";
  constexpr char const NODETYPE_ATTR[] = "NodeType";
  constexpr char const NODEID_TAG[] = "NodeId";
  constexpr char const COMMENT_TAG[] = "Comment";
  constexpr char const PRIORITY_TAG[] = "Priority";
  constexpr char const USING_MUTEX_TAG[] = "UsingMutex";

  constexpr char const INTERFACE_TAG[] = "Interface";
  constexpr char const IN_TAG[] = "In";
  constexpr char const INOUT_TAG[] = "InOut";

  constexpr char const BODY_TAG[] = "NodeBody";
  constexpr char const ASSN_TAG[] = "Assignment";
  constexpr char const RHS_TAG[] = "RHS";
  constexpr char const LIBRARYNODECALL_TAG[] = "LibraryNodeCall";
  constexpr char const ALIAS_TAG[] = "Alias";
  constexpr char const NODE_PARAMETER_TAG[] = "NodeParameter";
  constexpr char const NODELIST_TAG[] = "NodeList";

  constexpr char const VAR_DECLS_TAG[] = "VariableDeclarations";
  constexpr char const DECL_VAR_TAG[] = "DeclareVariable";
  constexpr char const NAME_TAG[] = "Name";
  constexpr char const TYPE_TAG[] = "Type";
  constexpr char const INITIALVAL_TAG[] = "InitialValue";
  constexpr char const DECL_ARRAY_TAG[] = "DeclareArray";
  constexpr char const MAX_SIZE_TAG[] = "MaxSize";
  constexpr char const ANY_VAL[] = "Any";
  constexpr char const DECLARE_MUTEX_TAG[] = "DeclareMutex";

  // condition names
  constexpr char const CONDITION_SUFFIX[] = "Condition";
  constexpr char const START_CONDITION_TAG[] = "StartCondition";
  constexpr char const REPEAT_CONDITION_TAG[] = "RepeatCondition";
  constexpr char const PRE_CONDITION_TAG[] = "PreCondition";
  constexpr char const POST_CONDITION_TAG[] = "PostCondition";
  constexpr char const INVARIANT_CONDITION_TAG[] = "InvariantCondition";
  constexpr char const END_CONDITION_TAG[] = "EndCondition";
  constexpr char const EXIT_CONDITION_TAG[] = "ExitCondition";
  constexpr char const SKIP_CONDITION_TAG[] = "SkipCondition";

  constexpr char const NODEREF_TAG[] = "NodeRef";
  constexpr char const DIR_ATTR[] = "dir";
  constexpr char const PARENT_VAL[] = "parent";
  constexpr char const CHILD_VAL[] = "child";
  constexpr char const SIBLING_VAL[] = "sibling";
  constexpr char const SELF_VAL[] = "self";

  constexpr char const STATEVAL_TAG[] = "NodeStateValue";

  constexpr char const TIMEPOINT_TAG[] = "Timepoint";
  constexpr char const START_VAL[] = "START";
  constexpr char const END_VAL[] = "END";

  constexpr char const ARGS_TAG[] = "Arguments";

  constexpr char const LOOKUPNOW_TAG[] = "LookupNow";
  constexpr char const LOOKUPCHANGE_TAG[] = "LookupOnChange";
  constexpr char const PAIR_TAG[] = "Pair";
  constexpr char const TOLERANCE_TAG[] = "Tolerance";

  constexpr char const CMD_TAG[] = "Command";
  constexpr char const UPDATE_TAG[] = "Update";

  constexpr char const RESOURCE_TAG[] = "Resource";
  constexpr char const RESOURCE_LIST_TAG[] = "ResourceList";
  constexpr char const RESOURCE_NAME_TAG[] = "ResourceName";
  constexpr char const RESOURCE_PRIORITY_TAG[] = "ResourcePriority";
  constexpr char const RESOURCE_UPPER_BOUND_TAG[] = "ResourceUpperBound";
  constexpr char const RESOURCE_RELEASE_AT_TERMINATION_TAG[] = "ResourceReleaseAtTermination";

  constexpr char const ARRAYVAR_TAG[] = "ArrayVariable";
  constexpr char const ARRAYELEMENT_TAG[] = "ArrayElement";
  constexpr char const INDEX_TAG[] = "Index";

  constexpr char const STRING_VAL_TAG[] = "StringValue";
  constexpr char const ARRAY_VAL_TAG[] = "ArrayValue";

  // Type literal tags
  constexpr char const BOOLEAN_VAL_TAG[] = "BooleanValue";
  constexpr char const INTEGER_VAL_TAG[] = "IntegerValue";
  constexpr char const REAL_VAL_TAG[] = "RealValue";
  constexpr char const DATE_VAL_TAG[] = "DateValue";
  constexpr char const DURATION_VAL_TAG[] = "DurationValue";
  constexpr char const NODE_STATE_VAL_TAG[] = "NodeStateValue";
  constexpr char const NODE_OUTCOME_VAL_TAG[] = "NodeOutcomeValue";
  constexpr char const NODE_FAILURE_VAL_TAG[] = "NodeFailureValue";
  constexpr char const NODE_COMMAND_HANDLE_VAL_TAG[] = "NodeCommandHandleValue";

  // 'Noise words' added for use by analysis tools
  constexpr char const ASSUME_TAG[] = "Assume";
  constexpr char const DESIRE_TAG[] = "Desire";
  constexpr char const EXPECT_TAG[] = "Expect";

} // namespace PLEXIL

#endif // PLEXIL_SCHEMA_HH
