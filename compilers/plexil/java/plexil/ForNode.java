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

// Structure is:
// (FOR_KYWD (VARIABLE_DECLARATION ...) <endtest> <loopvarupdate> (ACTION ...))

public class ForNode extends NodeTreeNode implements PlexilNode
{
    // We establish a subcontext for the loop,
    // otherwise the loop variable winds up declared once as a normal variable,
    // again as the loop variable.
    private NodeContext m_loopContext = null;

    public ForNode(Token t)
    {
        super(t);
    }

	public ForNode(ForNode n)
	{
		super(n);
        m_loopContext = n.m_loopContext;
	}

    @Override
	public Tree dupNode()
	{
		return new ForNode(this);
	}

    //
    // PlexilNode API
    //
    
    @Override
    public NodeContext getLocalContext()
    {
        return m_loopContext;
    }

    @Override
    public boolean inheritsParentContext()
    {
        return false;
    }

    // Always creates a new NodeContext.
    private void initializeLoopBodyContext(NodeContext parentContext)
    {
        m_loopContext = new NodeContext(parentContext, m_nodeId + "__FOR_BODY");
    }

    @Override
    protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
    {
        super.earlyCheckSelf(parentContext, state); // NodeTreeNode method
        initializeLoopBodyContext(parentContext);
    }

    @Override
    protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
    {
        for (PlexilTreeNode child : this.getChildren())
            child.earlyCheck(m_loopContext, state);
    }


    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        for (PlexilTreeNode child : this.getChildren())
            child.check(m_loopContext, state);

        VariableDeclNode loopVarDecl = (VariableDeclNode) this.getChild(0);
        PlexilDataType loopVarType = loopVarDecl.getVariableType();
        PlexilTreeNode typeNode = loopVarDecl.getChild(0);
        if (!loopVarType.isNumeric()) {
            state.addDiagnostic(typeNode,
                                  "\"for\" loop variable type is not numeric",
                                  Severity.ERROR);
        }
        ExpressionNode whileTest = (ExpressionNode) this.getChild(1);
        if (whileTest.getDataType() != PlexilDataType.BOOLEAN_TYPE) {
            state.addDiagnostic(whileTest,
                                "\"for\" loop test expression is not Boolean",
                                  Severity.ERROR);
        }
        ExpressionNode loopVarUpdate = (ExpressionNode) this.getChild(2);
        if (!loopVarUpdate.getDataType().isNumeric()) {
            state.addDiagnostic(typeNode,
                                  "\"for\" loop variable update expression is not a numeric expression",
                                  Severity.ERROR);
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        super.constructXMLBase(root);

        Element loopVar = root.createElement("LoopVariable");
        m_xml.appendChild(loopVar);
        // Get the declaration from the VariableName, not the declaration AST
        VariableName var = ((VariableDeclNode) this.getChild(0)).getVariableName();
        loopVar.appendChild(var.makeDeclarationXML(root));

        Element condition = root.createElement("Condition");
        m_xml.appendChild(condition);
        condition.appendChild(this.getChild(1).getXML(root));

        Element loopVarUpdate = root.createElement("LoopVariableUpdate");
        m_xml.appendChild(loopVarUpdate);
        loopVarUpdate.appendChild(this.getChild(2).getXML(root));

        Element action = root.createElement("Action");
        m_xml.appendChild(action);
        action.appendChild(this.getChild(3).getXML(root));
    }

    @Override
    protected String getXMLElementName() { return "For"; }

}
