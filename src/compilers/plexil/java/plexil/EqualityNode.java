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

import plexil.PlexilTreeNode;

public class EqualityNode extends ExpressionNode
{
	public EqualityNode(Token t)
	{
		super(t);
		m_dataType = PlexilDataType.BOOLEAN_TYPE;
	}

	/**
	 * @brief Check the expression for type consistency.
	 * @return true if consistent, false otherwise.
	 */
	public boolean checkTypeConsistency(NodeContext context, CompilerState myState)
	{
		// TODO: add diagnostics
		PlexilDataType lhsType = ((ExpressionNode) this.getChild(0)).getDataType();
		PlexilDataType rhsType = ((ExpressionNode) this.getChild(1)).getDataType();
		if (lhsType.isNumeric())
			return rhsType.isNumeric();
		return lhsType == rhsType;
	}

	/**
	 * @brief Construct the XML representing this part of the parse tree, and store it in m_xml.
	 */
	protected void constructXML()
	{
		super.constructXML();
		m_xml.addChild(this.getChild(0).getXML());
		m_xml.addChild(this.getChild(1).getXML());
	}

	protected String getXMLElementName()
	{
		String result;
		if (this.getType() == PlexilLexer.DEQUALS)
			result = "EQ";
		else
			result = "NE";

		PlexilDataType lhsType = ((ExpressionNode) this.getChild(0)).getDataType();
		if (lhsType == PlexilDataType.INTEGER_TYPE
			|| lhsType == PlexilDataType.REAL_TYPE)
			result = result + "Numeric";
		else if (lhsType == PlexilDataType.STRING_TYPE
				 || lhsType == PlexilDataType.BOOLEAN_TYPE)
			result = result + lhsType.typeName();
		else
			result = result + "Internal";
		return result;
	}

}
