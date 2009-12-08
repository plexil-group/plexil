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

import plexil.PlexilDataType;
import net.n3.nanoxml.*;

public class PlexilASTNode extends antlr.CommonAST
{
    // Needed for serializable
    private static final long serialVersionUID = -5891094434092392884L;
    private String filename = "";
    private int line = -1;
    private int column = -1;
    private PlexilDataType dataType = null;

    public PlexilASTNode() 
    {
	super();
    }

    public PlexilASTNode(antlr.Token tok) 
    {
	initialize(tok);
    }

    public PlexilDataType getDataType()
    {
	return dataType;
    }

    public void setDataType(PlexilDataType dt)
    {
	dataType = dt;
    }

    // returns true if the subtree represents an assignable quantity
    // (e.g. user variable, array reference)

    // default method
    public boolean isAssignable()
    {
        return false;
    }

    /**
     * Creation date: (4/9/2000 3:01:47 AM)
     * @author: Andrew Bachmann
     * 
     * @return int
     */
    public int getColumn() 
    {
	return column;
    }

    /**
     * Creation date: (4/9/2000 2:58:31 AM)
     * @author: Andrew Bachmann
     * 
     * @return java.lang.String
     */
    public java.lang.String getFilename() 
    {
	return filename;
    }

    /**
     * Creation date: (4/9/2000 2:58:31 AM)
     * @author: Andrew Bachmann
     * 
     * @return int
     */
    public int getLine() 
    {
	return line;
    }

    public void initialize(int t, String txt) 
    {
	super.initialize(t, txt);
    }

    public void initialize(antlr.collections.AST t) 
    {
	super.initialize(t);
	setLine(((PlexilASTNode)t).getLine());
	setColumn(((PlexilASTNode)t).getColumn());
	setFilename(((PlexilASTNode)t).getFilename());
    }

    public void initialize(antlr.Token tok) 
    {
	super.initialize(tok);
	setLine(tok.getLine());
	setColumn(tok.getColumn());
    }

    /**
     * Creation date: (4/9/2000 3:01:47 AM)
     * @author: Andrew Bachmann
     * 
     * @param newColumn int
     */
    protected void setColumn(int newColumn) 
    {
	column = newColumn;
    }

    /**
     * Creation date: (4/9/2000 2:58:31 AM)
     * @author: Andrew Bachmann
     * 
     * @param newFilename java.lang.String
     */
    protected void setFilename(java.lang.String newFilename) 
    {
	filename = newFilename;
    }

    /**
     * Creation date: (4/9/2000 2:58:31 AM)
     * @author: Andrew Bachmann
     * 
     * @param newLine int
     */
    protected void setLine(int newLine) 
    {
	line = newLine;
    }

    // default method

    public IXMLElement getXmlElement()
    {
	return null;
    }

    // default method

    public IXMLElement getXmlElement(String elementType)
    {
	return null;
    }

}
