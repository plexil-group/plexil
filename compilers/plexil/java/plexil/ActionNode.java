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

// ActionNode is a wrapper around an executable PLEXIL statement.
// It mainly serves as a place to hang a NodeId.

// Structure is:
// (ACTION NCName? baseAction)

public class ActionNode extends PlexilTreeNode
{
    //
    // Constructors
    //

    public ActionNode(Token t)
    {
        super(t);
    }

	public ActionNode(ActionNode n)
	{
		super(n);
	}

    public boolean hasNodeId()
    {
        return this.getChild(0).getType() == PlexilLexer.NCNAME;
    }

    public String getNodeId()
    {
        if (this.getChild(0).getType() != PlexilLexer.NCNAME)
            return null;
        return this.getChild(0).getText();
    }

    @Override
	public Tree dupNode()
	{
		return new ActionNode(this);
	}

    @Override
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
        // Look at the first child
        PlexilTreeNode firstChild = this.getChild(0);
        // If supplied, get the node ID
        if (hasNodeId()) {
            String nodeId = getNodeId();
            // Check that node ID is locally unique
            if (context.isChildNodeId(nodeId)) {
                state.addDiagnostic(firstChild,
                                    "Node ID \"" + nodeId + "\" defined more than once in this context",
                                    Severity.ERROR);
                state.addDiagnostic(context.getChildNodeId(nodeId),
                                    "Original definition of node ID \"" + nodeId + "\" is here",
                                    Severity.NOTE);
            }
            else {
                context.addChildNodeId(firstChild);
            }
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        // Get XML body from last child
        PlexilTreeNode child = this.getChild(this.getChildCount() - 1);
        m_xml = child.getXML(root);

        this.addSourceLocatorAttributes();

        // Insert Node ID element if child didn't already have one
        // and we have a NodeId.
        // *** Shouldn't be necessary any more -- see NodeTreeNode.initializeNodeId() ***
        if (DOMUtils.getFirstElementNamed(m_xml, "NodeId") == null
            && hasNodeId()) {
            Element nodeIdElt = root.createElement("NodeId");
            nodeIdElt.appendChild(root.createTextNode(getNodeId()));
            m_xml.insertBefore(nodeIdElt, m_xml.getFirstChild());
        }
    }

    /**
     * @brief Add new source locator attributes to m_xml, or replace the existing ones.
     * @note Use the first child's location as ours.
     */

    @Override
    protected void addSourceLocatorAttributes()
    {
        // Use location of first child
        Token t = getChild(0).getToken();
        m_xml.setAttribute("LineNo", String.valueOf(t.getLine()));
        m_xml.setAttribute("ColNo", String.valueOf(t.getCharPositionInLine()));
    }

}

