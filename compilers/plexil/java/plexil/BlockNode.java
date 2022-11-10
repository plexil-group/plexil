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

import plexil.xml.DOMUtils;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
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
    extends NodeTreeNode
{
    // Components of the block
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

    // Only check whether this block has an explicit node ID.
    @Override
    public boolean hasNodeId()
    {
        // Check ancestry
        PlexilTreeNode parent = this.getParent();
        if (parent == null)
            return false; // shouldn't happen

        // If directly wrapped in an action, check the action.
        if (parent.getType() == PlexilLexer.ACTION)
            return parent.hasNodeId();

        // Nope
        return false;
    }

    // A BlockNode always establishes its own context.
    @Override
    public boolean inheritsParentContext()
    {
        return false;
    }

    @Override
    public void initializeContext(NodeContext parentContext)
    {
        this.initializeNodeId();
        m_context = new NodeContext(parentContext, m_nodeId);
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

        boolean prioritySeen = false;

        // Partition the children by type
        for (PlexilTreeNode child : this.getChildren()) {
            switch (child.getType()) {
            case PlexilLexer.COMMENT_KYWD:
                // ignore until code generation time
                break;

            case PlexilLexer.IN_KYWD:
            case PlexilLexer.IN_OUT_KYWD:
            case PlexilLexer.ARRAY_VARIABLE_DECLARATION:
            case PlexilLexer.VARIABLE_DECLARATION:
            case PlexilLexer.MUTEX_KYWD:
            case PlexilLexer.USING_KYWD:
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
                if (prioritySeen) {
                    state.addDiagnostic(child,
                                        "In node " + m_context.getNodeName()
                                        + " Multiple Priority attributes",
                                        Severity.ERROR);
                } else {
                    prioritySeen = true;
                }
                break;

            case PlexilLexer.ACTION:
            case PlexilLexer.ASSIGNMENT:
            case PlexilLexer.BLOCK:
            case PlexilLexer.CHECKED_SEQUENCE_KYWD:
            case PlexilLexer.COMMAND:
            case PlexilLexer.CONCURRENCE_KYWD:
            case PlexilLexer.DO_KYWD:
            case PlexilLexer.FOR_KYWD:
            case PlexilLexer.IF_KYWD:
            case PlexilLexer.LBRACE:
            case PlexilLexer.LIBRARY_CALL_KYWD:
            case PlexilLexer.ON_COMMAND_KYWD:
            case PlexilLexer.ON_MESSAGE_KYWD:
            case PlexilLexer.SEQUENCE_KYWD:
            case PlexilLexer.SYNCHRONOUS_COMMAND_KYWD:
            case PlexilLexer.TRY_KYWD:
            case PlexilLexer.UNCHECKED_SEQUENCE_KYWD:
            case PlexilLexer.UPDATE_KYWD:
            case PlexilLexer.WAIT_KYWD:
            case PlexilLexer.WHILE_KYWD:
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

    @Override
    protected String getXMLElementName()
    {
        if (m_body.isEmpty())
            return "Node";
        if (this.getType() == PlexilLexer.LBRACE)
            return "Sequence";
        return this.getToken().getText();
    }

    protected void constructXML(Document root)
    {
        Node bodyStart = null; // where to insert declarations etc.

        if (isSimpleNode()) {
            // Get base XML from child
            PlexilTreeNode firstChild = m_body.get(0);
            m_xml = firstChild.getXML(root);
            if (hasNodeId() || this.getChildCount() > 1)
                addSourceLocatorAttributes(); // Reset source locators to our location
            Element nodeIdElt = DOMUtils.getFirstElementNamed(m_xml, "NodeId");
            if (nodeIdElt != null) {
                // Skip over the node ID
                bodyStart = nodeIdElt.getNextSibling();
            }
            else {
                bodyStart = m_xml.getFirstChild();
            }
        }
        else {
            this.constructXMLBase(root); // for effect
            if (m_body.isEmpty()) {
                // Empty node
                m_xml.setAttribute("NodeType", "Empty");
            }
            else {
                // Insert body as sequence of children
                for (PlexilTreeNode n : m_body)
                    m_xml.appendChild(n.getXML(root));
                // Skip over node ID
                bodyStart = m_xml.getFirstChild().getNextSibling();
            }
        }

        // Add comment, if supplied
        PlexilTreeNode commentTree =
            (PlexilTreeNode) this.getFirstChildWithType(PlexilLexer.COMMENT_KYWD);
        if (commentTree != null) {
            Element comment = commentTree.getXML(root);

            // Extract the string from the StringLiteralNode
            // and substitute it for the StringValue element
            Node stringText = comment.getFirstChild().getFirstChild();
            comment.replaceChild(stringText, comment.getFirstChild());
            if (bodyStart == null)
                m_xml.appendChild(comment);
            else
                m_xml.insertBefore(comment, bodyStart);
        }

        // Get variables from context
        List<VariableName> localVars = new ArrayList<VariableName>();
        List<InterfaceVariableName> inVars = new ArrayList<InterfaceVariableName>();
        List<InterfaceVariableName> inOutVars = new ArrayList<InterfaceVariableName>();
        m_context.getNodeVariables(localVars, inVars, inOutVars);

        if (!inVars.isEmpty() || !inOutVars.isEmpty()) {
            Element intfc = root.createElement("Interface");
            if (bodyStart == null)
                m_xml.appendChild(intfc);
            else
                m_xml.insertBefore(intfc, bodyStart);
            if (!inVars.isEmpty()) {
                Element inXML = root.createElement("In");
                intfc.appendChild(inXML);
                for (InterfaceVariableName iv : inVars)
                    inXML.appendChild(iv.makeDeclarationXML(root));
            }
            if (!inOutVars.isEmpty()) {
                Element inOutXML = root.createElement("InOut");
                intfc.appendChild(inOutXML);
                for (InterfaceVariableName iov : inOutVars)
                    inOutXML.appendChild(iov.makeDeclarationXML(root));
            }
        }
        if (!localVars.isEmpty() || !m_context.getMutexes().isEmpty()) {
            Element decls = root.createElement("VariableDeclarations");
            if (bodyStart == null)
                m_xml.appendChild(decls);
            else
                m_xml.insertBefore(decls, bodyStart);
            for (VariableName v : localVars)
                decls.appendChild(v.makeDeclarationXML(root));
            for (MutexName v : m_context.getMutexes())
                decls.appendChild(v.makeDeclarationXML(root));
        }

        // Add mutex references
        if (!m_context.getMutexRefs().isEmpty()) {
            Element mtx = root.createElement("UsingMutex");
            if (bodyStart == null)
                m_xml.appendChild(mtx);
            else
                m_xml.insertBefore(mtx, bodyStart);
            for (MutexName m : m_context.getMutexRefs()) {
                Element ref = m.asReference(root);
                // TODO: Add source locators
                mtx.appendChild(ref);
            }
        }

        // Add conditions
        for (PlexilTreeNode n : m_conditions) {
            if (bodyStart == null)
                m_xml.appendChild(n.getXML(root));
            else
                m_xml.insertBefore(n.getXML(root), bodyStart);
        }

        // Add priority, if supplied
        PlexilTreeNode priority =
            (PlexilTreeNode) this.getFirstChildWithType(PlexilLexer.PRIORITY_KYWD);
        if (priority != null) {
            if (bodyStart == null)
                m_xml.appendChild(priority.getXML(root));
            else
                m_xml.insertBefore(priority.getXML(root), bodyStart);
        }

        if (isCommandNode() && !m_resources.isEmpty()) {
            // Add command resources, if required
            // Has to be done here because CommandNode represents the naked command
            // and resources are in the surrounding braces
            Element rlist = root.createElement("ResourceList");
            for (PlexilTreeNode n : m_resources)
                rlist.appendChild(n.getXML(root));
            Element nodeBody = DOMUtils.getFirstElementNamed(m_xml, "NodeBody");
            Element commandXml = DOMUtils.getFirstElementNamed(nodeBody, "Command");
            commandXml.insertBefore(rlist, commandXml.getFirstChild());
        }
    }

    public boolean isSimpleNode()
    {
        // Don't collapse these block types because outcome semantics change
        if (this.getType() == PlexilLexer.UNCHECKED_SEQUENCE_KYWD
            || this.getType() == PlexilLexer.CONCURRENCE_KYWD)
            return false;

        if (m_body.size() != 1)
            return false;

        PlexilTreeNode firstChild = m_body.get(0);
        if (firstChild.getType() == PlexilLexer.ACTION) {
            if (this.hasNodeId() && firstChild.hasNodeId())
                return false; // both parent & child have node IDs
            firstChild = firstChild.getChild(firstChild.getChildCount() - 1);
        }

        switch (firstChild.getType()) {
            // Primitives
        case PlexilLexer.ASSIGNMENT:
        case PlexilLexer.COMMAND:
        case PlexilLexer.UPDATE_KYWD:
        case PlexilLexer.LIBRARY_CALL_KYWD:
            return true;

            // Compound
        case PlexilLexer.DO_KYWD:
        case PlexilLexer.FOR_KYWD:
        case PlexilLexer.IF_KYWD:
        case PlexilLexer.ON_COMMAND_KYWD:
        case PlexilLexer.ON_MESSAGE_KYWD:
        case PlexilLexer.SYNCHRONOUS_COMMAND_KYWD:
        case PlexilLexer.TRY_KYWD:
        case PlexilLexer.WHILE_KYWD:
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
        if (action.getType() == PlexilLexer.COMMAND) {
            return true;
        }
        if (action.getType() == PlexilLexer.ACTION) {
            return action.getChild(action.getChildCount() - 1).getType() == PlexilLexer.COMMAND;
        }
        return false;
    }

}
