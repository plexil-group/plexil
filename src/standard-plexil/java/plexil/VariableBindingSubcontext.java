// -*- Mode: Java; -*-

/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package plexil;

import java.util.Iterator;
import java.util.Vector;
import antlr.collections.AST;
import net.n3.nanoxml.IXMLElement;

import plexil.PlexilVariableName;
import plexil.PlexilInterfaceVariableName;

//
// A class to represent a variable binding context of an Extended PLEXIL form (e.g. For).
// Only handles local variable operations; defers everything else to its parent.
//

//
// *** To do:
//

public class VariableBindingSubcontext
    extends PlexilNodeContext
{
    public VariableBindingSubcontext(PlexilNodeContext previous, String name)
    {
	super(previous, name);
    }

    public void addChildNode(PlexilNodeContext child)
    {
	parentContext.addChildNode(child);
    }

    public boolean isLibraryNode()
    {
	return parentContext.isLibraryNode();
    }

    // Defer to parent because this isn't really a "node"
    public String generateChildNodeName()
    {
	return parentContext.generateChildNodeName();
    }

    //
    // Resources and resource priority
    //

    public Vector<AST> getResources()
    {
        return parentContext.getResources();
    }

    public void addResource(AST resourceAST)
    {
        parentContext.addResource(resourceAST);
    }

    public AST getResourcePriorityAST()
    {
        return parentContext.getResourcePriorityAST();
    }

    public void setResourcePriorityAST(AST priority)
    {
        parentContext.setResourcePriorityAST(priority);
    }

    public IXMLElement getResourcePriorityXML()
    {
        return parentContext.getResourcePriorityXML();
    }

    public void setResourcePriorityXML(IXMLElement priority)
    {
        parentContext.setResourcePriorityXML(priority);
    }

    public void declareInterfaceVariable(String name, 
					 boolean isInOut, 
					 PlexilDataType typ)
	throws antlr.SemanticException
    {
	throw new antlr.SemanticException("declareInterfaceVariable: Internal error: can't add interface variables to a subnode context!");
    }

    

}
