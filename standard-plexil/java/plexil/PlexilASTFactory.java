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

import antlr.ASTFactory;
import antlr.collections.AST;

// *** TO DO:
//  - Delegate XML generation to some nodes

public class PlexilASTFactory extends ASTFactory 
{
    private String location = null;
    /**
     * Creation date: (4/9/2000 3:25:07 AM)
     * @author: Andrew Bachmann
     * 
     * @param location Object
     */
    public PlexilASTFactory(Object location) 
    {
	super();
	if (location instanceof java.io.File) 
	    {
		setLocation(((java.io.File)location).getName());
	    } 
	else if (location instanceof java.net.URL) 
	    {
		setLocation(((java.net.URL)location).toExternalForm());
	    }	
	else if (location instanceof java.io.FileDescriptor) 
	    {
		setLocation(((java.io.FileDescriptor)location).toString());
	    } 
	else if (location instanceof java.io.InputStream) 
	    {
		setLocation(((java.io.InputStream)location).toString());
	    } 
	else if (location instanceof java.io.Reader) 
	    {
		setLocation(((java.io.Reader)location).toString());
	    } 
	else
	    {
		setLocation(location.toString());
	    }
	initializeNodeTypeMapping();
    }

    public PlexilASTFactory() 
    {
	super();
	initializeNodeTypeMapping();
    }

    protected void initializeNodeTypeMapping()
    {
	setASTNodeClass("plexil.PlexilASTNode");
    }

    //must override all creates that do not delegate to other creates!
	
    /**
     * Create a new empty AST node of type PlexilASTNode
     */
    public AST create(Class c) 
    {
	AST t = super.create(c);
	if (t instanceof PlexilASTNode)
	    {
		((PlexilASTNode)t).setFilename(getLocation());
	    }
	return t;
    }

    /**
     * Creation date: (4/9/2000 3:25:28 AM)
     * @author: Andrew Bachmann
     * 
     * @return String
     */
    public String getLocation() 
    {
	return location;
    }

    /**
     * Creation date: (4/9/2000 3:25:28 AM)
     * @author: Andrew Bachmann
     * 
     * @param newLocation String
     */
    protected void setLocation(String newLocation) 
    {
	location = newLocation;
    }

}
