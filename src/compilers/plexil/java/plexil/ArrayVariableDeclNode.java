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

import net.n3.nanoxml.*;

public class ArrayVariableDeclNode extends VariableDeclNode
{

	public ArrayVariableDeclNode(Token t)
	{
		super(t);
	}

	// format is (ARRAY_VARIABLE_DECLARATION element-type NCNAME INT initialValue?)
	public boolean check(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		PlexilTreeNode varNameNode = this.getChild(1);
		String varName = varNameNode.getText();

		// N.B. type is restricted syntactically so *should be* no chance of error here
		PlexilDataType elementType = PlexilDataType.findByName(this.getChild(0).getText());
		PlexilDataType arrayType = elementType.arrayType();
        if (arrayType == null) {
			myState.addDiagnostic(this.getChild(0),
								  "For array variable \"" + varName
								  + "\": Element type " + elementType.typeName()
								  + " has no associated array type",
								  Severity.ERROR);
			success = false;
		}

		// Check max size for non-negative integer
		LiteralNode sizeNode = (LiteralNode) this.getChild(2);
		String sizeString = sizeNode.getText();
		int size = LiteralNode.parseIntegerValue(sizeString);
		if (size < 0) {
			myState.addDiagnostic(sizeNode,
								  "For array variable \"" + varName
								  + "\": size " + sizeString + " is negative",
								  Severity.ERROR);
			success = false;
		}

		// If supplied, check initial value for type conflict
		// Track success of this check separately
		ExpressionNode initValNode = null;
		boolean initValSuccess = true;
		if (this.getChildCount() > 3) {
			// N.B. we assume this is a LiteralNode,
			// but ExpressionNode supports the required method
			// and allows the syntax to be generalized later.
			initValNode = (ExpressionNode) this.getChild(3);
			if (initValNode.getDataType().isArray()) {
				if (!initValNode.assumeType(arrayType, myState)) {
					// Just mark it as failed
					// assumeType() already reported the cause of the failure 
					initValSuccess = false;
				}

				// TODO: check size < max
				if (initValNode.getChildCount() > size) {
					myState.addDiagnostic(initValNode,
										  "For array variable \"" + varName
										  + "\": Array size is " + Integer.toString(size)
										  + ", but initial value has " + Integer.toString(initValNode.getChildCount())
										  + " elements", 
										  Severity.ERROR);
					initValSuccess = false;
				}
			}
			// Check scalar init value type consistency
			else if (!initValNode.assumeType(elementType, myState)) {
				myState.addDiagnostic(initValNode,
									  "For array variable \"" + varName
									  + "\": Initial value's type, "
									  + initValNode.getDataType().typeName()
									  + ", does not match array element type "
									  + elementType.typeName(),
									  Severity.ERROR);
				initValSuccess = false;
			}
			// Perform general semantic check of init value
			initValSuccess = initValNode.check(context, myState) && success;
		}

		// Add variable to context, even if initial value failed check,
		// to support further semantic checking in the plan
		if (success) {
			// Can fail in event of name conflict
			m_variable = context.declareArrayVariable(this,
													  varNameNode,
													  arrayType,
													  sizeString,
													  initValSuccess ? initValNode : null);
			if (m_variable == null)
				success = false;
		}
		m_passedCheck = success && initValSuccess;
		return m_passedCheck;
	}

}
