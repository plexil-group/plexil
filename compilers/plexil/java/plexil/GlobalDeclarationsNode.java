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

public class GlobalDeclarationsNode extends PlexilTreeNode
{
    private GlobalContext m_context = null;

    public GlobalDeclarationsNode(Token t)
    {
        super(t);
    }

	public GlobalDeclarationsNode(GlobalDeclarationsNode n)
	{
		super(n);
	}

    @Override
	public Tree dupNode()
	{
		return new GlobalDeclarationsNode(this);
	}

    // Capture the global context and check that it is in fact global.
    @Override
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
        if (context.isGlobalContext())
            m_context = (GlobalContext) context;
        else
            state.addDiagnostic(this,
                                "Internal error: GlobalDeclarationsNode context is not global!",
                                Severity.FATAL);
    }

    // Add global mutex declarations.
    @Override
    protected void constructXML(Document root)
    {
        super.constructXML(root);
        if (m_context != null) {
            for (MutexName mn : m_context.getMutexes())
                m_xml.appendChild(mn.makeDeclarationXML(root));
        }
    }

    @Override
    protected void addSourceLocatorAttributes()
    {
        // set source locator to location of 1st child (?)
        if (m_xml != null) {
            PlexilTreeNode firstChild = this.getChild(0);
            if (firstChild != null) {
                m_xml.setAttribute("LineNo",
                                   String.valueOf(firstChild.getToken().getLine()));
                m_xml.setAttribute("ColNo",
                                   String.valueOf(firstChild.getToken().getCharPositionInLine()));
            }
        }
    }
   
    protected String getXMLElementName()
    {
        return "GlobalDeclarations";
    }

}
