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

			// Block types
		case PlexilLexer.BLOCK:
		case PlexilLexer.CONCURRENCE_KYWD:
		case PlexilLexer.TRY_KYWD:
		case PlexilLexer.UNCHECKED_SEQUENCE_KYWD:
			return new BlockNode(payload);

			// Other syntactic features
		case PlexilLexer.FOR_KYWD:
			return new ForNode(payload);
		case PlexilLexer.IF_KYWD:
			return new IfNode(payload);
		case PlexilLexer.WHILE_KYWD:
			return new WhileNode(payload);

			// Internal tokens
		case PlexilLexer.VARIABLE_DECLARATION:
			return new VariableDeclNode(payload);

			// Literals
		case PlexilLexer.TRUE_KYWD:
		case PlexilLexer.FALSE_KYWD:
		case PlexilLexer.INT:
		case PlexilLexer.DOUBLE:
			return new LiteralNode(payload);

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

	// This seems to be of more use in tree parsing.
	public Object dupNode(Object t)
	{
		System.err.println("PlexilTreeAdaptor.dupNode of " + t.getClass().getName());
		if (t instanceof ArithmeticOperatorNode)
			return new ArithmeticOperatorNode((ArithmeticOperatorNode) t);
		else if (t instanceof LogicalOperatorNode)
			return new LogicalOperatorNode((LogicalOperatorNode) t);
		else if (t instanceof ExpressionNode)
			return new ExpressionNode((ExpressionNode) t);
		else if (t instanceof BlockNode)
			return new BlockNode((BlockNode) t);
		else if (t instanceof PlexilTreeNode)
			return new PlexilTreeNode((PlexilTreeNode) t);
		else if (t instanceof CommonTree)
			return new PlexilTreeNode((CommonTree) t);
		else if (t instanceof Token)
			return new PlexilTreeNode((Token) t);
		else return null;
	}

}