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

import java.util.ArrayList;
import java.util.List;

public class PlexilTreeNode extends org.antlr.runtime.tree.CommonTree
{
    protected Element m_xml = null;

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

    @Override
	public Tree dupNode()
	{
		return new PlexilTreeNode(this);
	}

    // This supersedes, but does not override, the BaseTree method.
    public PlexilTreeNode getChild(int i)
    {
        return (PlexilTreeNode) super.getChild(i);
    }

    // This supersedes, but does not override, the BaseTree method.
    @SuppressWarnings("unchecked")
    public List<? extends PlexilTreeNode> getChildren()
    {
        List<? extends Object> kids = super.getChildren();
        if (kids == null)
            return new ArrayList<PlexilTreeNode>();
        return (List<? extends PlexilTreeNode>) kids;
    }


    // This supersedes, but does not override, the CommonTree method.
    public PlexilTreeNode getParent()
    {
        return (PlexilTreeNode) super.getParent();
    }



    //
    // Extensions
    //

    /**
     * @brief Get the containing name binding context for this branch of the parse tree.
     * @return A NodeContext instance, or the global context.
     * @note Used by the PlexilTreeTransforms post-processor.
     */
    public NodeContext getContext()
    {
        PlexilTreeNode parent = getParent();
        if (parent != null)
            return parent.getContext();
        else
            return GlobalContext.getGlobalContext();
    }

    /**
     * @brief Establish bindings and do initial checks in top-down order.
     */
    public void earlyCheck(NodeContext context, CompilerState state)
    {
        earlyCheckSelf(context, state);
        earlyCheckChildren(context, state);
    }

    /**
     * @brief Establish local bindings and do initial self checks.
     * @note This default method does nothing. Derived classes should override it.
     */
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
    }

    /**
     * @brief Establish bindings and do initial checks of this node's children.
     * @note Derived classes, especially those which establish binding contexts,
     *       should override this as applicable.
     */
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        for (PlexilTreeNode child : this.getChildren())
            child.earlyCheck(context, state);
    }

    /**
     * @brief Perform a recursive semantic check in bottom-up order.
     */
    public void check(NodeContext context, CompilerState state)
    {
        checkChildren(context, state);
        checkSelf(context, state);
    }

    /**
     * @brief Perform a semantic check of this node's requirements.
     * @note This is a default method. Derived classes should implement their own. 
     */
    protected void checkSelf(NodeContext context, CompilerState state)
    {
    }

    /**
     * @brief Perform semantic checks on the node's children.
     */
    protected void checkChildren(NodeContext context, CompilerState state)
    {
        for (PlexilTreeNode child : getChildren())
            child.check(context, state);
    }
	
    //* Returns the DOM representation of this part of the parse tree.
    public Element getXML()
    {
        if (m_xml == null)
            constructXML();
        return m_xml;
    }

    /**
     * @brief Construct the XML representing this part of the parse tree, and store it in m_xml.
     * @note This is a default method. Derived classes should extend or override it as required.
     */
    protected void constructXML()
    {
        constructXMLBase();
        for (PlexilTreeNode child : getChildren()) {
            Element childXml = child.getXML();
            if (childXml != null)
                m_xml.appendChild(childXml);
        }
    }

    /**
     * @brief Construct the XML element representing this part of the parse tree, and store it in m_xml.
     * @note This is a default method. Derived classes should extend or override it as required.
     */
    protected void constructXMLBase()
    {
        m_xml = CompilerState.newElement(this.getXMLElementName());
        this.addSourceLocatorAttributes();
    }

    /**
     * @brief Get the string to use for the XML element's name.
     * @return A non-null String.
     * @note This is a base method. Derived classes should override it as required.
     */
    protected String getXMLElementName()
    {
        return this.getText();
    }

    /**
     * @brief Add new source locator attributes to m_xml, or replace the existing ones.
     */
    protected void addSourceLocatorAttributes()
    {
        if (m_xml != null) {
            m_xml.setAttribute("LineNo", String.valueOf(this.getLine()));
            m_xml.setAttribute("ColNo", String.valueOf(this.getCharPositionInLine()));
        }
    }

}
