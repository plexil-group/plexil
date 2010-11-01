// Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

import net.n3.nanoxml.*;

public class ActionNode extends PlexilTreeNode
{
	protected NodeContext m_context = null;
	protected String m_nodeId = null;

	//
	// Constructors
	//
	public ActionNode()
	{
		super();
	}

	// *** N.B. m_xml is ignored.
	public ActionNode(ActionNode node)
	{
		super(node);
	}

	public ActionNode(PlexilTreeNode node)
	{
		super(node);
	}

	public ActionNode(int ttype, NodeContext context)
	{
		super(new CommonToken(ttype, "ACTION"));
		m_context = context;
	}

	public ActionNode(Token t)
	{
		super(t);
	}

	public ActionNode(CommonTree node)
	{
		super(node);
	}

	public boolean checkSelf(NodeContext context, CompilerState myState)
	{
		boolean success = true;

		// If supplied, check that node ID is unique in parent
		PlexilTreeNode firstChild = this.getChild(0);
		if (firstChild.getType() == PlexilLexer.NCNAME) {
			m_nodeId = firstChild.getText();
			if (m_context.getParentContext().getChildContext(firstChild.getText()) != m_context) {
				myState.addDiagnostic(firstChild,
									  "Node ID \"" + firstChild.getText() + "\" is multiply defined",
									  Severity.ERROR);
				success = false;
			}
		}
		else {
			// Set node ID from context (which got it at constructor time from parent)
			m_nodeId = m_context.getNodeName();
		}

		// TODO: other checks?

		return success;
	}

	// Use local node context for kids
	public boolean checkChildren(NodeContext context, CompilerState myState)
	{
		return super.checkChildren(m_context, myState);
	}

	protected void constructXML()
	{
		// Get XML from last child
		PlexilTreeNode child = this.getChild(this.getChildCount() - 1);
		m_xml = child.getXML();
		// Insert Node ID element
		IXMLElement nodeIdElt = new XMLElement("NodeId");
		nodeIdElt.setContent(m_nodeId);
		((XMLElement) m_xml).insertChild(nodeIdElt, 0);
		// TODO: Construct and insert variable declarations from context
		
	}

}

