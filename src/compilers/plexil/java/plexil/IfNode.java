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

public class IfNode extends PlexilTreeNode
{
	public IfNode(Token t)
	{
		super(t);
	}

	public IfNode(IfNode n)
	{
		super(n);
	}

	public boolean checkSelf(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		if (((ExpressionNode) this.getChild(0)).getDataType() != PlexilDataType.BOOLEAN_TYPE) {
			myState.addDiagnostic(this.getChild(0),
								  "If expression is not Boolean",
								  Severity.ERROR);
			success = false;
		}
		// Check remaining clauses
		int i = 2;
		while (i < this.getChildCount() && this.getChild(i).getType() == PlexilLexer.ELSEIF_KYWD) {
			if (((ExpressionNode)this.getChild(i + 1)).getDataType() != PlexilDataType.BOOLEAN_TYPE) {
				myState.addDiagnostic(this.getChild(i+1),
									  "ElseIf expression is not Boolean",
									  Severity.ERROR);
				success = false;
			}
			i += 3;
		}
		return success;
	}

	protected void constructXML()
	{
		super.constructXML(); // constructs "If" element

		// Insert if-condition
		IXMLElement condition = new XMLElement("Condition");
		condition.addChild(this.getChild(0).getXML());
		m_xml.addChild(condition);

		// Insert then clause
		IXMLElement consequent = new XMLElement("Then");
		consequent.addChild(this.getChild(1).getXML());
		m_xml.addChild(consequent);

		// Handle ElseIf
		IXMLElement parent = m_xml;
		int i = 2;
		while (i < this.getChildCount() && this.getChild(i).getType() == PlexilLexer.ELSEIF_KYWD) {
			// Construct new IF
			IXMLElement elseifNode = new XMLElement("If");
			parent.addChild(elseifNode);
			// add source locators
			// TODO: add FileName attribute
			elseifNode.setAttribute("LineNo", String.valueOf(this.getChild(i).getLine()));
			elseifNode.setAttribute("ColNo", String.valueOf(this.getChild(i).getCharPositionInLine()));

			// Insert elseif-condition
			condition = new XMLElement("Condition");
			condition.addChild(this.getChild(i+1).getXML());
			elseifNode.addChild(condition);

			// Insert then clause
			consequent = new XMLElement("Then");
			consequent.addChild(this.getChild(i+2).getXML());
			elseifNode.addChild(consequent);

			i += 3;
			parent = elseifNode;
		}

		// Handle final else, if any
		if (i < this.getChildCount()) {
			IXMLElement elseNode = new XMLElement("Else");
			parent.addChild(elseNode);
			elseNode.addChild(this.getChild(i).getXML());
		}
	}

	protected String getXMLElementName() { return "If"; }

}