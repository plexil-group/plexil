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

public class ArrayReferenceNode extends VariableNode
{
	private VariableNode m_variableNode = null;
	private ExpressionNode m_index = null;

	public ArrayReferenceNode(Token t)
	{
		super(t);
	}

	public void preCheckInit()
	{
		m_variableNode = (VariableNode) this.getChild(0);
		m_index = (ExpressionNode) this.getChild(1);
	}

	public boolean check(NodeContext context, CompilerState myState)
	{
		preCheckInit();

		boolean success = true;

		// Check that variable is declared array
		success = m_variableNode.check(context, myState);
		PlexilDataType varType = m_variableNode.getDataType();
		if (varType != PlexilDataType.VOID_TYPE) { // i.e. var failed defined check
			if (!varType.isArray()) {
				myState.addDiagnostic(this.getChild(0),
									  "Variable \"" + m_variableNode.getText() + "\" is not an array variable",
									  Severity.ERROR);
				success = false;
			}
			m_dataType = varType.arrayElementType();
		}

		// Check index expression

		success = m_index.check(context, myState) && success;
		if (!m_index.getDataType().isNumeric()) {
			myState.addDiagnostic(this.getChild(1),
								  "Array index expression is not numeric",
								  Severity.ERROR);
			success = false;
		}

		m_passedCheck = success;
		return success;
	}

	protected void constructXML()
	{
		super.constructXML();

		// Construct variable name element
		IXMLElement var = new XMLElement("Name");
		var.setContent(this.getChild(0).getText());
		m_xml.addChild(var);

		// Construct index
		IXMLElement idx = new XMLElement("Index");
		idx.addChild(this.getChild(1).getXML());
		m_xml.addChild(idx);
	}

	protected String getXMLElementName() { return "ArrayElement"; }

	public boolean checkAssignable(CompilerState myState)
	{
		// defer to variable
		return m_variableNode.checkAssignable(myState);
	}

}
