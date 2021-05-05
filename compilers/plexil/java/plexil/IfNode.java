// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

import java.util.Vector;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Element;

public class IfNode extends NodeTreeNode
{
    public IfNode(Token t)
    {
        super(t);
    }

	public IfNode(IfNode n)
	{
		super(n);
        m_context = n.m_context;
	}

    @Override
	public Tree dupNode()
	{
		return new IfNode(this);
	}

    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        super.checkChildren(parentContext, state); // NodeTreeNode method

        // Assert Boolean type on tests
        if (!((ExpressionNode) getChild(0)).assumeType(PlexilDataType.BOOLEAN_TYPE, state)) {
            state.addDiagnostic(getChild(0),
                                "If test expression is not Boolean",
                                Severity.ERROR);
        }
        int nkids = getChildCount();
        for (int i = 2; i < nkids; i += 2) {
            if (nkids - i > 1) {
                // Elseif test & clause
                if (!((ExpressionNode) getChild(i)).assumeType(PlexilDataType.BOOLEAN_TYPE, state)) {
                    state.addDiagnostic(getChild(i),
                                        "ElseIf test expression is not Boolean",
                                        Severity.ERROR);
                }
            }
        }
    }

    @Override
    protected void constructXML()
    {
        super.constructXMLBase(); // constructs "If" element

        // Insert if-condition
        Element condition = CompilerState.newElement("Condition");
        condition.appendChild(getChild(0).getXML());
        m_xml.appendChild(condition);

        // Insert then clause
        Element consequent = CompilerState.newElement("Then");
        consequent.appendChild(getChild(1).getXML());
        m_xml.appendChild(consequent);

        int nkids = getChildCount();
        if (nkids == 2)
            return;
        
        int i = 2;
        for (; i < nkids; i += 2) {
            if (nkids - i > 1) {
                // Insert ElseIf clause(s)
                Element elseIfClause = CompilerState.newElement("ElseIf");
                Element elseIfCondition = CompilerState.newElement("Condition");
                elseIfCondition.appendChild(getChild(i).getXML());
                elseIfClause.appendChild(elseIfCondition);
                Element elseIfConsequent = CompilerState.newElement("Then");
                elseIfConsequent.appendChild(getChild(i + 1).getXML());
                elseIfClause.appendChild(elseIfConsequent);
                m_xml.appendChild(elseIfClause);
            }
            else {
                // insert final Else clause
                Element elseClause = CompilerState.newElement("Else");
                elseClause.appendChild(getChild(i).getXML());
                m_xml.appendChild(elseClause);
            }
        }
    }

    protected String getXMLElementName() { return "If"; }

}
