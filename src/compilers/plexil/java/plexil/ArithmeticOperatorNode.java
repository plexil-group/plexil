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

public class ArithmeticOperatorNode extends ExpressionNode
{
	public ArithmeticOperatorNode(ArithmeticOperatorNode a)
	{
		super(a);
	}

	public ArithmeticOperatorNode(Token t)
	{
		super(t);
	}

	public ArithmeticOperatorNode(int ttype)
	{
		super(new CommonToken(ttype, getTokenString(ttype)));
	}

	// I *think* this is only needed for ABS_KYWD...
	private static String getTokenString(int ttype)
	{
		switch (ttype) {
		case PlexilLexer.ABS_KYWD:
			return "ABS";
		default:
			return "UNKNOWN_ARITH_OP_" + String.valueOf(ttype);
		}
	}

	// May have 1 or more args
	// Special cases:
	//  PLUS with 2 or more args can have strings

	// Computes m_dataType as a side effect.
	public boolean checkTypeConsistency(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		if (this.getChildCount() == 1) {
			// Unary + or - must be numeric
			ExpressionNode operand = (ExpressionNode) this.getChild(0);
			if (!operand.getDataType().isNumeric()) {
				// TODO: improve message for things like absolute value
				myState.addDiagnostic(operand,
									  "The operand to the " + this.getToken().getText() + " operator is not numeric",
									  Severity.ERROR);
				success = false;
			}
			if (this.getType() == PlexilLexer.SQRT_KYWD)
				m_dataType = PlexilDataType.REAL_TYPE;
			else
				m_dataType = operand.getDataType();
		}
		else if (this.getType() == PlexilLexer.PLUS) {
			for (int i = 0; i < this.getChildCount(); i++) {
				ExpressionNode operand = (ExpressionNode) this.getChild(i);
				PlexilDataType otype = operand.getDataType();
				if (i == 0) {
					if (otype.isNumeric() || otype == PlexilDataType.STRING_TYPE) {
						// any of these types is OK
						m_dataType = operand.getDataType();
					}
					else {
						myState.addDiagnostic(operand,
											  "The first operand to the " + this.getToken().getText()
											  + " operator is not a numeric type or a string",
											  Severity.ERROR);
						success = false;
					}
				}
				else {
					// following parameters must match first op's type
					if (otype == m_dataType) {
						// no action needed
					}
					else if (otype.isNumeric() && m_dataType.isNumeric()) {
						// Perform type contagion
						if (m_dataType == PlexilDataType.INTEGER_TYPE
							&& otype == PlexilDataType.REAL_TYPE) {
							// Result is real
							m_dataType = otype;
						}
					}
					else {
						myState.addDiagnostic(operand,
											  "Operands to the " + this.getToken().getText() + " operator have inconsistent types",
											  Severity.ERROR);
						m_dataType = PlexilDataType.VOID_TYPE;
						success = false;
					}
				}
			}
		}
		else {
			// General case - all numeric
			// Implement type contagion
			for (int i = 0; i < this.getChildCount(); i++) {
				ExpressionNode operand = (ExpressionNode) this.getChild(i);
				PlexilDataType otype = operand.getDataType();
				if (i == 0) {
					if (otype.isNumeric()) {
						m_dataType = operand.getDataType();
					}
					else {
						myState.addDiagnostic(operand,
											  "The first operand to the " + this.getToken().getText()
											  + " operator is not a numeric type",
											  Severity.ERROR);
						success = false;
					}
				}
				else {
					// following parameters must all be numeric
					if (!otype.isNumeric()) {
						myState.addDiagnostic(operand,
											  "The operand to the " + this.getToken().getText() + " operator is not a numeric type",
											  Severity.ERROR);
						m_dataType = PlexilDataType.VOID_TYPE;
						success = false;
					}
					else if (m_dataType == PlexilDataType.INTEGER_TYPE
							 && otype == PlexilDataType.REAL_TYPE) {
						// Result is real
						m_dataType = otype;
					}
				}
			}
		}
		return success;
	}

	public void constructXML()
	{
		super.constructXML();
		for (int i = 0; i < this.getChildCount(); i++) {
			m_xml.addChild(this.getChild(i).getXML());
		}
	}

	public String getXMLElementName()
	{
		switch (this.getType()) {
		case PlexilLexer.PLUS:
			if (m_dataType == PlexilDataType.STRING_TYPE)
				return "Concat";
			else return "ADD";
			
		case PlexilLexer.MINUS:
			return "SUB";

		case PlexilLexer.ASTERISK:
			return "MUL";

		case PlexilLexer.SLASH:
			return "DIV";

		case PlexilLexer.PERCENT:
			return "MOD";

		default:
			return this.getToken().getText();
		}
	}

}