// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

import java.util.Set;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class WhileNode extends NodeTreeNode
{
    public WhileNode(Token t)
    {
        super(t);
    }

    public WhileNode(WhileNode n)
    {
        super(n);
    }

	public Tree dupNode()
	{
		return new WhileNode(this);
	}


    /**
     * @brief Semantic check.
     */
    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        super.checkChildren(parentContext, state); // NodeTreeNode method

        ExpressionNode whileTest = (ExpressionNode) this.getChild(0);
        if (!whileTest.assumeType(PlexilDataType.BOOLEAN_TYPE, state)) {
            state.addDiagnostic(whileTest,
                                "\"while\" test expression is not Boolean",
                                Severity.ERROR);
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        super.constructXMLBase(root);

        Element condition = root.createElement("Condition");
        m_xml.appendChild(condition);
        condition.appendChild(this.getChild(0).getXML(root));

        Element action = root.createElement("Action");
        m_xml.appendChild(action);
        action.appendChild(this.getChild(1).getXML(root));
    }

    protected String getXMLElementName() { return "While"; }

}
