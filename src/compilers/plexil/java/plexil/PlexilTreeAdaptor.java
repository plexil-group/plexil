// Copyright (c) 2006-2010, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
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

package plexil;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

public class PlexilTreeAdaptor extends org.antlr.runtime.tree.CommonTreeAdaptor
{
	public PlexilTreeAdaptor()
	{
		super();
	}

	public Object create(Token payload)
	{
		if (payload == null)
			return new PlexilTreeNode(payload);

		switch (payload.getType()) {
			// Arithmetic operators
		case PlexilLexer.ABS_KYWD:
		case PlexilLexer.ASTERISK:
		case PlexilLexer.MINUS:
		case PlexilLexer.PERCENT:
		case PlexilLexer.PLUS:
		case PlexilLexer.SLASH:
		case PlexilLexer.SQRT_KYWD:
			return new ArithmeticOperatorNode(payload);

			// Logical operators
		case PlexilLexer.AND_KYWD:
		case PlexilLexer.NOT_KYWD:
		case PlexilLexer.OR_KYWD:
		case PlexilLexer.XOR_KYWD:
			return new LogicalOperatorNode(payload);

			// Equality comparisons
		case PlexilLexer.DEQUALS:
		case PlexilLexer.NEQUALS:
			return new EqualityNode(payload);

			// Numeric comparisons
		case PlexilLexer.GREATER:
		case PlexilLexer.GEQ:
		case PlexilLexer.LESS:
		case PlexilLexer.LEQ:
			return new RelationalNode(payload);

			// Lookups
		case PlexilLexer.LOOKUP_KYWD:
		case PlexilLexer.LOOKUP_NOW_KYWD:
		case PlexilLexer.LOOKUP_ON_CHANGE_KYWD:
			return new LookupNode(payload);

			// Block types
		case PlexilLexer.BLOCK:
		case PlexilLexer.CONCURRENCE_KYWD:
		case PlexilLexer.TRY_KYWD:
		case PlexilLexer.UNCHECKED_SEQUENCE_KYWD:
			return new BlockNode(payload);

			// Conditions
		case PlexilLexer.END_CONDITION_KYWD:
		case PlexilLexer.INVARIANT_CONDITION_KYWD:
		case PlexilLexer.POST_CONDITION_KYWD:
		case PlexilLexer.PRE_CONDITION_KYWD:
		case PlexilLexer.REPEAT_CONDITION_KYWD:
		case PlexilLexer.SKIP_CONDITION_KYWD:
		case PlexilLexer.START_CONDITION_KYWD:
			return new ConditionNode(payload);

			// Other syntactic features
		case PlexilLexer.ARGUMENT_LIST:
			return new ArgumentListNode(payload);

		case PlexilLexer.FOR_KYWD:
			return new ForNode(payload);

		case PlexilLexer.IF_KYWD:
			return new IfNode(payload);

		case PlexilLexer.IN_KYWD:
		case PlexilLexer.IN_OUT_KYWD:
			return new InterfaceDeclNode(payload);

		case PlexilLexer.LIBRARY_ACTION_KYWD:
			return new LibraryDeclarationNode(payload);

		case PlexilLexer.ON_COMMAND_KYWD:
		case PlexilLexer.ON_MESSAGE_KYWD:
			return new OnCommandNode(payload);

		case PlexilLexer.PARAMETERS:
			return new ParameterSpecNode(payload);

		case PlexilLexer.RETURNS_KYWD:
			return new ReturnSpecNode(payload);

		case PlexilLexer.WAIT_KYWD:
			return new WaitNode(payload);

		case PlexilLexer.WHILE_KYWD:
			return new WhileNode(payload);

			// Internal tokens
		case PlexilLexer.ACTION:
			return new ActionNode(payload);

		case PlexilLexer.ASSIGNMENT:
			return new AssignmentNode(payload);

		case PlexilLexer.ARRAY_REF:
			return new ArrayReferenceNode(payload);

		case PlexilLexer.ARRAY_VARIABLE_DECLARATION:
			return new ArrayVariableDeclNode(payload);

		case PlexilLexer.VARIABLE_DECLARATION:
			return new VariableDeclNode(payload);

			// Literals
		case PlexilLexer.TRUE_KYWD:
		case PlexilLexer.FALSE_KYWD:
		case PlexilLexer.INT:
		case PlexilLexer.DOUBLE:
			return new LiteralNode(payload);

		case PlexilLexer.ARRAY_LITERAL:
			return new ArrayLiteralNode(payload);

		case PlexilLexer.STRING:
			return new StringLiteralNode(payload);

		default:
			return new PlexilTreeNode(payload);
		}
	}

	public Object errorNode(TokenStream input,
							Token start,
							Token stop,
							RecognitionException e)
	{
		PlexilErrorNode t = new PlexilErrorNode(input, start, stop, e);
		return t;
	}

}