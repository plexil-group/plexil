// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

import org.w3c.dom.Element;

public class ArrayReferenceNode extends VariableNode
{
    public ArrayReferenceNode(Token t)
    {
        super(t);
    }

	public ArrayReferenceNode(ArrayReferenceNode n)
	{
		super(n);
	}

	public Tree dupNode()
	{
		return new ArrayReferenceNode(this);
	}

    // Override VariableNode method
    @Override
    public void earlyCheck(NodeContext context, CompilerState state)
    {
        earlyCheckChildren(context, state);
        ExpressionNode arrayNode = (ExpressionNode) this.getChild(0);
        // Set source locators from array expression
        this.getToken().setLine(arrayNode.getLine());
        this.getToken().setCharPositionInLine(arrayNode.getCharPositionInLine());

        PlexilDataType arrayType = arrayNode.getDataType();
        if (arrayType.isArray())
            m_dataType = arrayType.arrayElementType();
        else {
            state.addDiagnostic(arrayNode,
                                "Expression is not an array",
                                Severity.ERROR);
        }
    }

    public void checkTypeConsistency(NodeContext context, CompilerState state)
    {
        // Check array expression type
        ExpressionNode arrayNode = (ExpressionNode) this.getChild(0);
        PlexilDataType arrayType = arrayNode.getDataType();
        
        if (arrayType == PlexilDataType.UNKNOWN_ARRAY_TYPE
            || arrayType == PlexilDataType.ERROR_TYPE) {
            // type error already detected elsewhere
            }
        else if (arrayType == PlexilDataType.ANY_TYPE)
            m_dataType = PlexilDataType.ANY_TYPE;

        else if (arrayType.isArray())
            m_dataType = arrayType.arrayElementType(); // may be redundannt with earlyCheck()

        else
            state.addDiagnostic(arrayNode,
                                "Expression is not an array",
                                Severity.ERROR);

        // Check index expression type
        ExpressionNode index = (ExpressionNode) this.getChild(1);
        if (index.getDataType() != PlexilDataType.INTEGER_TYPE) {
            state.addDiagnostic(index,
                                "Array index expression is not integral!",
                                Severity.ERROR);
        }
    }

    // N.B. Can't use super.constructXML because of conflict with VariableNode method
    @Override
    protected void constructXML()
    {
        this.constructXMLBase();

        // Construct array expression
        m_xml.appendChild(this.getChild(0).getXML());

        // Construct index
        Element idx = CompilerState.newElement("Index");
        idx.appendChild(this.getChild(1).getXML());
        m_xml.appendChild(idx);
    }

    @Override
    protected String getXMLElementName()
    {
        return "ArrayElement";
    }

    public boolean isAssignable()
    {
        // defer to the variable
        VariableNode variableNode = (VariableNode) this.getChild(0);
        return variableNode.isAssignable();
    }

    public void checkAssignable(NodeContext context, CompilerState state)
    {
        // defer to the variable
        VariableNode variableNode = (VariableNode) this.getChild(0);
        variableNode.checkAssignable(context, state);
    }

}
