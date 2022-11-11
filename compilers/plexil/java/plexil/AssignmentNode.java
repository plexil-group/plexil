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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import plexil.xml.DOMUtils;

public class AssignmentNode extends NodeTreeNode
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

    // Overrides NodeTreeNode method.
    protected void initializeNodeId()
    {
        // Check ancestry
        PlexilTreeNode parent = this.getParent();
        if (parent != null) {
            // If directly wrapped in an Action node, get its ID.
            if (parent instanceof ActionNode
                && ((ActionNode) parent).hasNodeId()) {
                m_nodeId = ((ActionNode) parent).getNodeId();
                return;
            }

            // If directly wrapped by a block, and we're its only body child,
            // check the block.
            if (parent instanceof BlockNode
                && ((BlockNode) parent).hasNodeId()
                && ((BlockNode) parent).isCollapsible()) { // i.e. we're an only child
                m_nodeId = ((BlockNode) parent).getNodeId();
                return;
            }

            // If this is actually a command node with a result variable, leave node ID empty.
            if (this.getChild(1) instanceof CommandNode) {
                m_nodeId = null;
                return;
            }
        }

        // Create one
        this.createNodeId(this.getToken().getText());
    }

    @Override
    protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
    {
        super.earlyCheckSelf(parentContext, state); // NodeTreeNode method

        Token t = this.getToken();
        PlexilTreeNode rhs = this.getChild(1);
        if (rhs instanceof CommandNode) {
            // Change the advertised type of this tree node to command
            t.setType(PlexilLexer.COMMAND);
            t.setLine(rhs.getLine());
            t.setCharPositionInLine(rhs.getCharPositionInLine());
        }
        else {
            // Set source locators to point to LHS
            PlexilTreeNode lhs = this.getChild(0);
            t.setLine(lhs.getLine());
            t.setCharPositionInLine(lhs.getCharPositionInLine());
        }
    }

    @Override
    protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
    {
        super.earlyCheckChildren(parentContext, state); // NodeTreeNode method

        // N.B. LHS can be an array reference
        ExpressionNode lhs = (ExpressionNode) this.getChild(0);
        lhs.checkAssignable(m_context, state);
    }

    // Assumption is that LHS is strongly typed.  RHS may not be.
    @Override
    protected void checkSelf(NodeContext parentContext, CompilerState state)
    {
        boolean invalidTypes = false;

        // N.B. LHS can be an array reference
        ExpressionNode lhs = (ExpressionNode) this.getChild(0);

        // N.B. RHS can be a CommandNode
        Expression rhs = (Expression) this.getChild(1);

        PlexilDataType rhsType = rhs.getDataType();
        if (rhsType == PlexilDataType.VOID_TYPE) {
            state.addDiagnostic(this.getChild(1),
                                "Expression on right-hand side of assignment has no return value",
                                Severity.ERROR);
            invalidTypes = true;
        }
        else if (!PlexilDataType.isValid(rhsType)) {
            state.addDiagnostic(this.getChild(1),
                                "Internal error: right-hand side of assignment has invalid type"
                                + rhsType.toString(),
                                Severity.FATAL);
            invalidTypes = true;
        }

        PlexilDataType lhsType = lhs.getDataType();
        if (!PlexilDataType.isValid(lhsType)) {
            state.addDiagnostic(lhs,
                                "Internal error: left-hand side of assignment has invalid type"
                                + lhsType.toString(),
                                Severity.FATAL);
            invalidTypes = true;
        }

        if (invalidTypes)
            return;

        if (lhsType == rhsType) {
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
                        state.addDiagnostic(this.getChild(1),
                                            "Can't assign an array of max size "
                                            + Integer.toString(rhsVar.getMaxSize())
                                            + " to an array variable of max size "
                                            + Integer.toString(lhsVar.getMaxSize()),
                                            Severity.ERROR);
                    }
                }
            }
        }
        else if (rhsType == PlexilDataType.ANY_TYPE) {
            state.addDiagnostic(this.getChild(1),
                                "Any-type expression coerced to " + lhsType.toString(),
                                Severity.WARNING);
        }
        // Type mismatch
        else if (lhsType != PlexilDataType.REAL_TYPE || !rhsType.isNumeric()) {
            state.addDiagnostic(this.getChild(1),
                                "Cannot assign expression of type " + rhsType.typeName()
                                + " to \"" + lhs.getText() 
                                + "\" of type " + lhsType.typeName(),
                                Severity.ERROR);
        }
    }

    static private String rhsElementName(PlexilDataType t)
    {
        if (t.isNumeric() || t.isTemporal())
            return "NumericRHS";
        if (t.isArray())
            return "ArrayRHS";
        if (t == PlexilDataType.STRING_TYPE)
            return "StringRHS";
        if (t == PlexilDataType.BOOLEAN_TYPE)
            return "BooleanRHS";
        else 
            return "ERROR_RHS";
    }

    @Override
    protected void constructXML(Document root)
    {
        PlexilTreeNode lhs = this.getChild(0);
        if (this.getChild(1) instanceof CommandNode) {
            // This is really a Command node, 
            // so get its XML, find Name element, and insert LHS in front of it.
            m_xml = this.getChild(1).getXML(root);
            Element body = DOMUtils.getFirstElementNamed(m_xml, "NodeBody");
            Element command = (Element) body.getFirstChild();
            Element tmp = DOMUtils.getFirstElementNamed(command, "Name");
            command.insertBefore(lhs.getXML(root), tmp);

            // set Command element source location to the loc'n of the LHS
            command.setAttribute("LineNo", String.valueOf(lhs.getLine()));
            command.setAttribute("ColNo", String.valueOf(lhs.getCharPositionInLine()));
        }
        else {
            super.constructXMLBase(root);
            m_xml.setAttribute("NodeType", "Assignment");

            Element assign = root.createElement("Assignment");
            // set source location to the loc'n of the LHS
            assign.setAttribute("LineNo", String.valueOf(lhs.getLine()));
            assign.setAttribute("ColNo", String.valueOf(lhs.getCharPositionInLine()));

            assign.appendChild(lhs.getXML(root));

            ExpressionNode rhs = (ExpressionNode) this.getChild(1);
            Element rhsXML = root.createElement(rhsElementName(rhs.getDataType()));
            rhsXML.appendChild(rhs.getXML(root));
            assign.appendChild(rhsXML);

            Element body = root.createElement("NodeBody");
            body.appendChild(assign);

            m_xml.appendChild(body);
        }
    }

    protected String getXMLElementName() { return "Node"; }

}
