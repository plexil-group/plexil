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
import org.w3c.dom.Node;

import plexil.xml.DOMUtils;

public class AssignmentNode extends PlexilTreeNode
{
    public AssignmentNode(Token t)
    {
        super(t);
    }

    public AssignmentNode(AssignmentNode n)
    {
        super(n);
    }

	public Tree dupNode()
	{
		return new AssignmentNode(this);
	}

    public void earlyCheck(NodeContext context, CompilerState state)
    {
        // N.B. LHS can be an array reference
        ExpressionNode lhs = (ExpressionNode) this.getChild(0);
        ExpressionNode rhs = (ExpressionNode) this.getChild(1);

        lhs.earlyCheck(context, state);
        lhs.checkAssignable(context, state);
        rhs.earlyCheck(context, state);

        Token t = this.getToken();
        if (rhs instanceof CommandNode) {
            // Change this to a command tree node
            t.setType(PlexilLexer.COMMAND);
            t.setLine(rhs.getLine());
            t.setCharPositionInLine(rhs.getCharPositionInLine());
        }
        else {
            // Set source locators to point to LHS
            t.setLine(lhs.getLine());
            t.setCharPositionInLine(lhs.getCharPositionInLine());
        }
    }

    public void check(NodeContext context, CompilerState state)
    {
        // N.B. LHS can be an array reference
        ExpressionNode lhs = (ExpressionNode) this.getChild(0);
        ExpressionNode rhs = (ExpressionNode) this.getChild(1);

        lhs.check(context, state); // for effect;
        PlexilDataType lhsType = lhs.getDataType();
        if (lhsType != null) {
            // LHS passed check
            rhs.assumeType(lhsType, state); // for effect
        }

        rhs.check(context, state); // for effect
        PlexilDataType rhsType = rhs.getDataType(); // is VOID_TYPE if RHS failed check or is command w/ no return value
        if (rhsType == PlexilDataType.VOID_TYPE) {
            state.addDiagnostic(rhs,
                                "Expression or command has no return value",
                                Severity.ERROR);
        }
        else if (lhsType != null) {
            if (lhsType == rhsType ||
                (lhsType == PlexilDataType.REAL_TYPE && rhsType.isNumeric())) {
                // All OK so far
                if (lhsType.isArray()) {
                    // Check array dimensions if possible
                    // *** TODO generalize LHS to array expr ***
                    // LHS must be array var
                    // RHS can only be command, lookup, or variable (at present)
                    VariableName lhsVar = ((VariableNode) lhs).getVariableName();
                    VariableName rhsVar = null;
                    if (rhs instanceof VariableNode)
                        rhsVar = ((VariableNode) rhs).getVariableName();
                    else if (rhs instanceof LookupNode) {
                        // Get return variable if known
                        GlobalDeclaration stateDecl = ((LookupNode) rhs).getState();
                        if (stateDecl != null)
                            rhsVar = stateDecl.getReturnVariable();
                    }
                    else if (rhs instanceof CommandNode) {
                        // Get return variable if known
                        GlobalDeclaration cmdDecl = ((CommandNode) rhs).getCommand();
                        if (cmdDecl != null)
                            rhsVar = cmdDecl.getReturnVariable();
                    }
                    if (lhsVar != null && rhsVar != null) {
                        if (lhsVar.getMaxSize() < rhsVar.getMaxSize()) {
                            state.addDiagnostic(rhs,
                                                "Can't assign an array of max size "
                                                + Integer.toString(rhsVar.getMaxSize())
                                                + " to an array variable of max size "
                                                + Integer.toString(lhsVar.getMaxSize()),
                                                Severity.ERROR);
                        }
                    }
                }
            }
            else {
                state.addDiagnostic(rhs,
                                    "Cannot assign expression of type " + rhsType.typeName()
                                    + " to \"" + lhs.getText() 
                                    + "\" of type " + lhsType.typeName(),
                                    Severity.ERROR);
            }
        }
    }

    @Override
    protected void constructXML()
    {
        PlexilTreeNode lhs = this.getChild(0);
        ExpressionNode rhs = (ExpressionNode) this.getChild(1);
        if (rhs.getType() == PlexilLexer.COMMAND) {
            // This is really a Command node, 
            // so find Name element and insert LHS in front of it
            m_xml = rhs.getXML();
            Element body = DOMUtils.getFirstElementNamed(m_xml, "NodeBody");
            Element command = (Element) body.getFirstChild();
            Element tmp = DOMUtils.getFirstElementNamed(command, "Name");
            command.insertBefore(lhs.getXML(), tmp);

            // set Command element source location to the loc'n of the LHS
            command.setAttribute("LineNo", String.valueOf(lhs.getLine()));
            command.setAttribute("ColNo", String.valueOf(lhs.getCharPositionInLine()));
        }
        else {
            super.constructXMLBase();
            m_xml.setAttribute("NodeType", "Assignment");

            Element assign = CompilerState.newElement("Assignment");
            // set source location to the loc'n of the LHS
            assign.setAttribute("LineNo", String.valueOf(lhs.getLine()));
            assign.setAttribute("ColNo", String.valueOf(lhs.getCharPositionInLine()));

            assign.appendChild(lhs.getXML());

            Element rhsXML = CompilerState.newElement(rhs.assignmentRHSElementName());
            rhsXML.appendChild(rhs.getXML());
            assign.appendChild(rhsXML);

            Element body = CompilerState.newElement("NodeBody");
            body.appendChild(assign);

            m_xml.appendChild(body);
        }
    }

    protected String getXMLElementName() { return "Node"; }

}
