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

import java.util.Vector;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import net.n3.nanoxml.*;

public class BlockNode extends PlexilTreeNode
{
	NodeContext m_context = null;

	PlexilTreeNode m_comment = null;
	Vector<PlexilTreeNode> m_declarations = new Vector<PlexilTreeNode>();
	Vector<PlexilTreeNode> m_conditions = new Vector<PlexilTreeNode>();
	Vector<PlexilTreeNode> m_attributes = new Vector<PlexilTreeNode>();
	Vector<PlexilTreeNode> m_resources = new Vector<PlexilTreeNode>();
	Vector<PlexilTreeNode> m_body = new Vector<PlexilTreeNode>();

	public BlockNode(BlockNode b)
	{
		super(b);
		// FIXME: should this copy context??
		m_context = b.m_context;
	}

	public BlockNode(Token t)
	{
		super(t);
	}

	public BlockNode(int ttype)
	{
		super(new CommonToken(ttype, "BLOCK"));
	}

	// N.B. Interface and variable decl's, and conditions, check themselves.
	public boolean checkSelf(NodeContext context, CompilerState myState)
	{
		m_context = context;
		partitionChildren();
		boolean success = true;

		// Check for duplicate conditions
		TreeSet<Integer> conditionsSeen = new TreeSet<Integer>();
		for (PlexilTreeNode c : m_conditions) {
			Integer condType = new Integer(c.getType());
			if (conditionsSeen.contains(condType)) {
				myState.addDiagnostic(c,
									  "Multiple \"" + c.getToken().getText() + "\" conditions specified",
									  Severity.ERROR);
				success = false;
			}
			else {
				conditionsSeen.add(condType);
			}
		}

		if (isCommandNode()) {
			// TODO: Resources legal, look for collisions
		}
		else {
			// Resources are NOT legal, flag them as errors
			for (PlexilTreeNode r : m_resources) {
				myState.addDiagnostic(r, 
									  "The \"" + r.getToken().getText() + "\" keyword is only valid for Command actions",
									  Severity.ERROR);
				success = false;
			}
		}

		// TODO: Check attributes
		for (PlexilTreeNode a : m_attributes) {
			// Check for collisions
		}

		return success;
	}

	private void partitionChildren()
	{
		m_declarations.clear();
		m_conditions.clear();
		m_attributes.clear();
		m_resources.clear();
		m_body.clear();

		if (this.getChildCount() > 0) {
			int i = 0;
			if (this.getChild(i).getType() == PlexilLexer.COMMENT_KYWD) {
				m_comment = this.getChild(i);
				i++;
			}
			for ( ; i < this.getChildCount(); i++) {
				PlexilTreeNode child = this.getChild(i);
				switch (child.getType()) {
				case PlexilLexer.IN_KYWD:
				case PlexilLexer.IN_OUT_KYWD:
				case PlexilLexer.VARIABLE_DECLARATIONS:
					m_declarations.add(child);
					break;

				case PlexilLexer.END_CONDITION_KYWD:
				case PlexilLexer.INVARIANT_CONDITION_KYWD:
				case PlexilLexer.POST_CONDITION_KYWD:
				case PlexilLexer.PRE_CONDITION_KYWD:
				case PlexilLexer.REPEAT_CONDITION_KYWD:
				case PlexilLexer.SKIP_CONDITION_KYWD:
				case PlexilLexer.START_CONDITION_KYWD:
					m_conditions.add(child);
					break;

				case PlexilLexer.RESOURCE_KYWD:
				case PlexilLexer.RESOURCE_PRIORITY_KYWD:
					m_resources.add(child);
					break;

				case PlexilLexer.PRIORITY_KYWD:
				case PlexilLexer.PERMISSIONS_KYWD:
					m_attributes.add(child);
					break;

				default:
					m_body.add(child);
					break;
				}
			}
		}
	}


	// N.B. Could add more cases to inherit XML from child.
	protected void constructXML()
	{
		// Construct basic XML
		// If body is 1 simple action, use its XML as a base
		if (isSimpleNode()) {
			m_xml = m_body.firstElement().getChild(0).getXML();
		}
		else {
			// Build XML wrapper
			if (m_body.isEmpty()) {
				// Empty node
				m_xml = new XMLElement("Node");
				m_xml.setAttribute("NodeType", "Empty");
			}
			else {
				if (this.getType() == PlexilLexer.BLOCK) {
					m_xml = new XMLElement("Sequence");
				}
				else {
					m_xml = new XMLElement(this.getToken().getText());
				}
			}
			this.addSourceLocatorAttributes();
		}

		// Add comment
		if (m_comment != null) {
			IXMLElement comment = m_comment.getChild(1).getXML();
			comment.setName("Comment");
			m_xml.addChild(comment);
		}

		Vector<VariableName> localVars = new Vector<VariableName>();
		Vector<InterfaceVariableName> inVars = new Vector<InterfaceVariableName>();
		Vector<InterfaceVariableName> inOutVars = new Vector<InterfaceVariableName>();
		m_context.getNodeVariables(localVars, inVars, inOutVars);
		if (!inVars.isEmpty() || !inOutVars.isEmpty()) {
			IXMLElement intfc = new XMLElement("Interface");
			m_xml.addChild(intfc);
			if (!inVars.isEmpty()) {
				IXMLElement inXML = new XMLElement("In");
				intfc.addChild(inXML);
				for (InterfaceVariableName iv : inVars)
					inXML.addChild(iv.makeDeclarationXML());
			}
			if (!inOutVars.isEmpty()) {
				IXMLElement inOutXML = new XMLElement("InOut");
				intfc.addChild(inOutXML);
				for (InterfaceVariableName iov : inOutVars)
					inOutXML.addChild(iov.makeDeclarationXML());
			}
		}
		if (!localVars.isEmpty()) {
			IXMLElement decls = new XMLElement("VariableDeclarations");
			m_xml.addChild(decls);
			for (VariableName v : localVars) {
				decls.addChild(v.makeDeclarationXML());
			}
		}

		// Add conditions
		for (PlexilTreeNode n : m_conditions) {
			m_xml.addChild(n.getXML());
		}

		// Add attributes
		for (PlexilTreeNode n : m_attributes) {
			m_xml.addChild(n.getXML());
		}

		// TODO: Add resources
		if (isCommandNode()) {
			for (PlexilTreeNode n : m_resources) {
			}
		}

		// Add children from body
		if (!isSimpleNode()) {
			for (PlexilTreeNode n : m_body) {
				m_xml.addChild(n.getXML());
			}
		}
	}

	private boolean isSimpleNode()
	{
		if (m_body.size() != 1) {
			return false;
		}

		PlexilTreeNode action = m_body.firstElement();
		// this should NEVER fail!
		if (action.getType() != PlexilLexer.ACTION) {
			return false;
		}
		if (action.getChildCount() != 1) {
			return false;
		}
		int bodyType = action.getChild(0).getType();
		switch (bodyType) {
		case PlexilLexer.ASSIGNMENT:
		case PlexilLexer.COMMAND:
		case PlexilLexer.LIBRARY_CALL_KYWD:
		case PlexilLexer.UPDATE_KYWD:
		case PlexilLexer.REQUEST_KYWD:
			return true;

		default:
			return false;
		}
	}

	private boolean isListNode()
	{
		return m_body.size() > 1;
	}

	private boolean isCommandNode()
	{
		if (m_body.size() != 1) {
			return false;
		}
		PlexilTreeNode action = m_body.firstElement();
		// this should NEVER fail!
		if (action.getType() != PlexilLexer.ACTION) {
			return false;
		}
		if (action.getChildCount() != 1) {
			return false;
		}
		return action.getChild(0).getType() == PlexilLexer.COMMAND;
	}

}