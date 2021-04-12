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

import plexil.xml.DOMUtils;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Element;
import org.w3c.dom.Node;

import java.util.ArrayList;
import java.util.List;

// A BlockNode represents a pair of braces around an arbitrary
// amount of PLEXIL code.

// A BlockNode always establishes a new NodeContext.

// A BlockNode can wrap zero or more Actions.
// If there is only one Action in the body, and it does not have
// its own node ID, it should inherit this BlockNode's context.
// Otherwise Action children should establish their own contexts.

public class BlockNode
    extends PlexilTreeNode
    implements PlexilNode
{
    // Variable name binding context
    private NodeContext m_context = null;

    // Components of the block
    private PlexilTreeNode m_comment = null;
    private PlexilTreeNode m_priority = null;
    private List<PlexilTreeNode> m_conditions = null;
    private List<PlexilTreeNode> m_resources = null;
    private List<PlexilTreeNode> m_body = null;

    public BlockNode(Token t)
    {
        super(t);
        m_conditions = new ArrayList<PlexilTreeNode>();
        m_resources = new ArrayList<PlexilTreeNode>();
        m_body = new ArrayList<PlexilTreeNode>();
    }

    public BlockNode(BlockNode n)
    {
        super(n);
        m_context = n.m_context;
		m_comment = n.m_comment;
		m_priority = n.m_priority;
		m_conditions = n.m_conditions;
		m_resources = n.m_resources;
		m_body = n.m_body;
    }

    @Override
	public Tree dupNode()
	{
		return new BlockNode(this);
	}

    //
    // PlexilNode API
    //

    public NodeContext getContext()
    {
        return m_context;
    }

    public boolean hasNodeId()
    {
        ActionNode parent = (ActionNode) this.getParent();
        return parent != null && parent.hasNodeId();
    }

    public boolean inheritsParentContext()
    {
        return false;
    }

    // Always creates a new NodeContext.
    public void initializeContext(NodeContext parentContext)
    {
        String nodeId = null;
        if (this.hasNodeId()) {
            nodeId = ((ActionNode) this.getParent()).getNodeId();
        }
        m_context = new NodeContext(parentContext, nodeId);
    }

    @Override
    protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
    {
        initializeContext(parentContext);
    }

    @Override
    protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
    {
        if (this.getChildCount() == 0)
            return; // nothing to do here

        // Partition the children by type
        for (PlexilTreeNode child : this.getChildren()) {
            switch (child.getType()) {
            case PlexilLexer.COMMENT_KYWD:
                m_comment = child;
                break;
            
            case PlexilLexer.IN_KYWD:
            case PlexilLexer.IN_OUT_KYWD:
            case PlexilLexer.VARIABLE_DECLARATIONS:
                // declarations add selves to context
                break;

            case PlexilLexer.END_CONDITION_KYWD:
            case PlexilLexer.EXIT_CONDITION_KYWD:
            case PlexilLexer.INVARIANT_CONDITION_KYWD:
            case PlexilLexer.POST_CONDITION_KYWD:
            case PlexilLexer.PRE_CONDITION_KYWD:
            case PlexilLexer.REPEAT_CONDITION_KYWD:
            case PlexilLexer.SKIP_CONDITION_KYWD:
            case PlexilLexer.START_CONDITION_KYWD:
                m_conditions.add(child);
                break;

            case PlexilLexer.RESOURCE_KYWD:
                m_resources.add(child);
                break;

            case PlexilLexer.PRIORITY_KYWD:
                if (m_priority != null) {
                    state.addDiagnostic(child, 
                                        "In node " + m_context.getNodeName()
                                        + " Multiple Priority attributes",
                                        Severity.ERROR);
                } else {
                    m_priority = child;
                }
                break;

            case PlexilLexer.ACTION:
                m_body.add(child);
                break;

            default:
                // should never happen (?)
                state.addDiagnostic(this,
                                    "Internal error: Unexpected child "
                                    + child.getToken().getText()
                                    + " in block",
                                    Severity.FATAL);
                break;
            }
        }

        // Now run their early checks
        for (PlexilTreeNode child : this.getChildren()) {
            child.earlyCheck(m_context, state);
        }
    }

    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        // Check for duplicate conditions
        List<Integer> conditionsSeen = new ArrayList<Integer>();
        for (PlexilTreeNode c : m_conditions) {
            Integer condType = Integer.valueOf(c.getType());
            if (conditionsSeen.contains(condType)) {
                state.addDiagnostic(c,
                                    "In node " + m_context.getNodeName()
                                    + ": Multiple \"" + c.getToken().getText()
                                    + "\" conditions specified",
                                    Severity.ERROR);
            }
            else {
                conditionsSeen.add(condType);
            }
        }

        if (isCommandNode()) {
            // Look for collisions between resource names
            // TODO
        }
        else {
            // Resources are NOT legal, flag them as errors
            for (PlexilTreeNode r : m_resources) {
                state.addDiagnostic(r, 
                                    "In node " + m_context.getNodeName()
                                    + "The \"" + r.getToken().getText()
                                    + "\" keyword is only valid for Command nodes",
                                    Severity.ERROR);
            }
        }

        // Recurse into children
        for (PlexilTreeNode child : this.getChildren())
            child.check(m_context, state);
    }

    protected void constructXML()
    {
        Node bodyStart = null;

        // Build basic node
        if (m_body.isEmpty()) {
            // Empty node
            m_xml = CompilerState.newElement("Node");
            m_xml.setAttribute("NodeType", "Empty");
        }
        else if (isSimpleNode()) {
            // Get base XML from child
            PlexilTreeNode action = m_body.get(0);
            if (firstChildHasNodeId()) {
                m_xml = action.getXML();  // get it with NodeId
                // Skip over the node ID
                bodyStart = m_xml.getFirstChild().getNextSibling();
            }
            else {
                m_xml = action.getChild(0).getXML(); // get bare node
                bodyStart = m_xml.getFirstChild();
            }
        }
        else {
            // 1 or more items in body
            if (this.getType() == PlexilLexer.LBRACE)
                m_xml = CompilerState.newElement("Sequence");
            else
                m_xml = CompilerState.newElement(this.getToken().getText());

            // Insert body as sequence of children
            for (PlexilTreeNode n : m_body)
                m_xml.appendChild(n.getXML());
            bodyStart = m_xml.getFirstChild();
        }

        // Set source locators after (potentially) hoisting child up
        this.addSourceLocatorAttributes();

        // Add comment
        if (m_comment != null) {
            Element comment = m_comment.getXML();

            // Extract the string from the StringLiteralNode
            // and substitute it for the StringValue element
            Node stringText = comment.getFirstChild().getFirstChild();
            comment.replaceChild(stringText, comment.getFirstChild());
            m_xml.insertBefore(comment, bodyStart);
        }

        // Get variables from context
        List<VariableName> localVars = new ArrayList<VariableName>();
        List<InterfaceVariableName> inVars = new ArrayList<InterfaceVariableName>();
        List<InterfaceVariableName> inOutVars = new ArrayList<InterfaceVariableName>();
        m_context.getNodeVariables(localVars, inVars, inOutVars);

        if (!inVars.isEmpty() || !inOutVars.isEmpty()) {
            Element intfc = CompilerState.newElement("Interface");
            m_xml.insertBefore(intfc, bodyStart);
            if (!inVars.isEmpty()) {
                Element inXML = CompilerState.newElement("In");
                intfc.appendChild(inXML);
                for (InterfaceVariableName iv : inVars)
                    inXML.appendChild(iv.makeDeclarationXML());
            }
            if (!inOutVars.isEmpty()) {
                Element inOutXML = CompilerState.newElement("InOut");
                intfc.appendChild(inOutXML);
                for (InterfaceVariableName iov : inOutVars)
                    inOutXML.appendChild(iov.makeDeclarationXML());
            }
        }
        if (!localVars.isEmpty()) {
            Element decls = CompilerState.newElement("VariableDeclarations");
            m_xml.insertBefore(decls, bodyStart);
            for (VariableName v : localVars)
                decls.appendChild(v.makeDeclarationXML());
        }

        // Add conditions
        for (PlexilTreeNode n : m_conditions)
            m_xml.insertBefore(n.getXML(), bodyStart);

        // Add priority
        if (m_priority != null)
            m_xml.insertBefore(m_priority.getXML(), bodyStart);

        if (!m_resources.isEmpty()) {
            if (isCommandNode()) {
                // Add command resources, if required
                // Has to be done here because CommandNode represents the naked command
                // and resources are in the surrounding braces
                Element rlist = CompilerState.newElement("ResourceList");
                for (PlexilTreeNode n : m_resources)
                    rlist.appendChild(n.getXML());
                Element nodeBody = DOMUtils.getFirstElementNamed(m_xml, "NodeBody");
                Element commandXml = DOMUtils.getFirstElementNamed(nodeBody, "Command");
                commandXml.insertBefore(rlist, commandXml.getFirstChild());
            } else {
                // TODO?: generate warning
            }
        }
    }

    private boolean firstChildHasNodeId()
    {
        if (m_body.isEmpty())
            return false;
        return 1 < m_body.get(0).getChildCount();
    }

    public boolean isSimpleNode()
    {
        // Don't collapse these block types because outcome semantics change
        if (this.getType() == PlexilLexer.UNCHECKED_SEQUENCE_KYWD
            || this.getType() == PlexilLexer.CONCURRENCE_KYWD)
            return false;

        if (m_body.size() != 1)
            return false;

        ActionNode action = (ActionNode) m_body.get(0); // (Action NCNAME? *)
        if (action == null)
            return false; // internal error, shouldn't happen
        if (this.hasNodeId() && action.hasNodeId())
            return false; // both parent & child have node IDs

        PlexilTreeNode subaction = action.getChild(action.getChildCount() - 1);
        switch (subaction.getType()) {
            // Primitives
        case PlexilLexer.ASSIGNMENT:
        case PlexilLexer.COMMAND:
        case PlexilLexer.UPDATE_KYWD:
        case PlexilLexer.LIBRARY_CALL_KYWD:
            return true;

            // Compound
        case PlexilLexer.IF_KYWD:
        case PlexilLexer.FOR_KYWD:
        case PlexilLexer.WHILE_KYWD:
        case PlexilLexer.ON_COMMAND_KYWD:
        case PlexilLexer.ON_MESSAGE_KYWD:
        case PlexilLexer.SYNCHRONOUS_COMMAND_KYWD:
            return true;

        default:
            return false;
        }
    }

    private boolean isCommandNode()
    {
        if (m_body.size() != 1) {
            return false;
        }
        PlexilTreeNode action = m_body.get(0);
        // this should NEVER fail!
        if (action.getType() != PlexilLexer.ACTION) {
            return false;
        }
        return action.getChild(action.getChildCount() - 1).getType() == PlexilLexer.COMMAND;
    }

}
