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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Element;

import java.util.List;

// Structure is
// ON_COMMAND_KYWD^ expression variableDeclaration* action)

public class OnCommandNode extends NodeTreeNode
{
    public OnCommandNode(Token t)
    {
        super(t);
    }

    public OnCommandNode(OnCommandNode n)
    {
        super(n);
    }

    @Override
	public Tree dupNode()
	{
		return new OnCommandNode(this);
	}

	// structure is:
	// ^(ON_COMMAND_KYWD expression paramsSpec? action)

    @Override
	protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
	{
        super.earlyCheckSelf(parentContext, state); // NodeTreeNode method

        // The Extended PLEXIL translator treats all VariableDeclaration forms
        // as parameters, so if a local (not interface) variable was declared
        // in the surrounding block, issue an error diagnostic.
        List<VariableName> vars = m_context.getLocalVariables();
        if (!vars.isEmpty()) {
            // Pick one declaration to use for error location purposes
            PlexilTreeNode declaration = vars.get(0).getDeclaration();
            state.addDiagnostic(declaration,
								"Local variables may not be declared in the block containing an OnCommand",
								Severity.ERROR);
        }
    }

	@Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
	{
        super.checkChildren(parentContext, state); // NodeTreeNode method

        ExpressionNode nameExp = (ExpressionNode) this.getChild(0);
		// Coerce name expression to string, if we can
        if (!nameExp.assumeType(PlexilDataType.STRING_TYPE, state)) {
            state.addDiagnostic(nameExp,
								"The name expression to the " + this.getToken().getText()
								+ " statement was not a string expression",
								Severity.ERROR);
		}
	}

    @Override
    protected void constructXML()
    {
        super.constructXMLBase(); // PlexilTreeNode method

        // If the OnCommand is surrounded by a block, the block
        // will output the parameter declarations for us
        if (!this.inheritsParentContext()) {
            // Output the parameters, if any
            List<VariableName> vars = m_context.getLocalVariables();
            if (!vars.isEmpty()) {
                Element decls = CompilerState.newElement("VariableDeclarations");
                m_xml.appendChild(decls);
                for (VariableName v : vars)
                    decls.appendChild(v.makeDeclarationXML());
            }
        }

        // Name next
        Element name = CompilerState.newElement ("Name");
		name.appendChild(this.getChild(0).getXML());
        m_xml.appendChild(name);

        // Body
        m_xml.appendChild(getBody().getXML());
    }

	private PlexilTreeNode getBody()
	{
        return this.getChild(this.getChildCount() - 1);
	}

}
