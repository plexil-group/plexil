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

public class PlexilTreeNode extends org.antlr.runtime.tree.CommonTree
{
	protected IXMLElement m_xml = null;
	protected boolean m_passedCheck = false;

	//
	// Constructors
	//
	public PlexilTreeNode()
	{
		super();
	}

	// *** N.B. m_xml is ignored.
	public PlexilTreeNode(PlexilTreeNode node)
	{
		super(node);
	}

	public PlexilTreeNode(Token t)
	{
		super(t);
	}

	public PlexilTreeNode(CommonTree node)
	{
		super(node);
	}


	//
	// Overrides
	//
	public PlexilTreeNode getChild(int i)
	{
		return (PlexilTreeNode) super.getChild(i);
	}


	//
	// Extensions
	//

	/**
	 * @brief Perform a recursive semantic check.
	 * @return true if check is successful, false otherwise.
	 */
	public boolean check(NodeContext context, CompilerState myState)
	{
		boolean success = checkChildren(context, myState);
		success = checkSelf(context, myState) && success;
		m_passedCheck = success;
		return success;
	}

	/**
	 * @brief Perform a semantic check of this node's requirements.
	 * @return true if check is successful, false otherwise.
	 * @note This is a default method. Derived classes should implement their own. 
	 */
	public boolean checkSelf(NodeContext context, CompilerState myState)
	{
		return true;
	}

	/**
	 * @brief Ask whether this node passed its check.
	 * @return true if check is successful, false otherwise.
	 */
	public boolean passedCheck() { return m_passedCheck; }

	/**
	 * @brief Perform semantic checks on the node's children.
	 * @return true if checks are successful, false otherwise.
	 */
	public boolean checkChildren(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		for (int i = 0; i < this.getChildCount(); i++) {
			success = this.getChild(i).check(context, myState) && success;
		}
		return success;
	}
	
	//* Returns the NanoXML representation of this part of the parse tree.
	public IXMLElement getXML()
	{
		if (m_xml == null) {
			constructXML();
		}
		return m_xml;
	}

	/**
	 * @brief Construct the XML representing this part of the parse tree, and store it in m_xml.
	 * @note This is a base method. Derived classes should extend or override it as required.
	 */
	protected void constructXML()
	{
		m_xml = new XMLElement(this.getXMLElementName());
		addSourceLocatorAttributes();
	}

	/**
	 * @brief Get the string to use for the XML element's name.
	 * @return A non-null String.
	 * @note This is a base method. Derived classes should override it as required.
	 */
	protected String getXMLElementName()
	{
		return this.getToken().getText();
	}

	/**
	 * @brief Add new source locator attributes to m_xml, or replace the existing ones.
	 */
	protected void addSourceLocatorAttributes()
	{
		if (m_xml != null) {
			// TODO: add FileName attribute
			m_xml.setAttribute("LineNo", String.valueOf(this.getLine()));
			m_xml.setAttribute("ColNo", String.valueOf(this.getCharPositionInLine()));
		}
	}

}