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
  char const *PLEXIL_PLAN_TAG = "PlexilPlan";
  char const *NODE_TAG = "Node";
  char const *NODEID_TAG = "NodeId";
  char const *PRIORITY_TAG = "Priority";
  char const *INTERFACE_TAG = "Interface";
  char const *VAR_DECLS_TAG = "VariableDeclarations";
  char const *DECL_VAR_TAG = "DeclareVariable";
  char const *IN_TAG = "In";
  char const *INOUT_TAG = "InOut";
  char const *VAR_TAG = "Variable";
  char const *TYPE_TAG = "Type";
  char const *MAXSIZE_TAG = "MaxSize";
  char const *DECL_TAG = "Declare";
  char const *VAL_TAG = "Value";
  char const *ARRAY_VAL_TAG = "ArrayValue";
  char const *INITIALVAL_TAG = "InitialValue";
  char const *ASSN_TAG = "Assignment";
  char const *BODY_TAG = "NodeBody";
  char const *RHS_TAG = "RHS";
  char const *NODELIST_TAG = "NodeList";
  char const *LIBRARYNODECALL_TAG = "LibraryNodeCall";
  char const *ALIAS_TAG = "Alias";
  char const *NODE_PARAMETER_TAG = "NodeParameter";
  char const *CMD_TAG = "Command";
  char const *NAME_TAG = "Name";
  char const *INDEX_TAG = "Index";
  char const *ARGS_TAG = "Arguments";
  char const *LOOKUPNOW_TAG = "LookupNow";
  char const *LOOKUPCHANGE_TAG = "LookupOnChange";
  char const *TOLERANCE_TAG = "Tolerance";
  char const *NODEREF_TAG = "NodeRef";
  char const *STATEVAL_TAG = "NodeStateValue";
  char const *TIMEPOINT_TAG = "Timepoint";
  char const *UPDATE_TAG = "Update";
  char const *PAIR_TAG = "Pair";
  char const *COND_TAG = "Condition";

  // condition names
  char const *START_CONDITION_TAG = "StartCondition";
  char const *REPEAT_CONDITION_TAG = "RepeatCondition";
  char const *PRE_CONDITION_TAG = "PreCondition";
  char const *POST_CONDITION_TAG = "PostCondition";
  char const *INVARIANT_CONDITION_TAG = "InvariantCondition";
  char const *END_CONDITION_TAG = "EndCondition";
  char const *EXIT_CONDITION_TAG = "ExitCondition";
  char const *SKIP_CONDITION_TAG = "SkipCondition";

  // FIXME: Use constants declared in ValueType.hh
  char const *INT_TAG = "Integer";
  char const *REAL_TAG = "Real";
  char const *BOOL_TAG = "Boolean";
  char const *STRING_TAG = "String";
  char const *DATE_TAG = "Date";
  char const *DURATION_TAG = "Date";
  char const *ARRAY_TAG = "Array";

  char const *NODE_OUTCOME_TAG = "NodeOutcome";
  char const *NODE_FAILURE_TAG = "NodeFailure";
  char const *NODE_STATE_TAG = "NodeState";
  char const *NODE_COMMAND_HANDLE_TAG = "NodeCommandHandle";

  char const *DECL_ARRAY_TAG = "DeclareArray";
  char const *ARRAYELEMENT_TAG = "ArrayElement";

  char const *NODETYPE_ATTR = "NodeType";
  char const *DIR_ATTR = "dir";

  char const *FILENAME_ATTR = "FileName";
  char const *LINENO_ATTR = "LineNo";
  char const *COLNO_ATTR = "ColNo";

  char const *PARENT_VAL = "parent";
  char const *CHILD_VAL = "child";
  char const *SIBLING_VAL = "sibling";
  char const *SELF_VAL = "self";

}
