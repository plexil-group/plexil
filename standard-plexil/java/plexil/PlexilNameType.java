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

/* Java 1.5 and up
public enum PlexilNameType
{
    UNDEFINED_NAME,
    NODE_NAME,
    VARIABLE_NAME,
    FUNCTION_NAME,
    COMMAND_NAME,
    EXTERNAL_STRUCT_NAME,
    STATE_NAME,
    PARAMETER_NAME
}
*/

    /* version backwardly compatible with Java 1.4 */
    import java.util.*;

public final class PlexilNameType
{
    private String id;
    private String prettyName;
    public final int ord;
    private PlexilNameType prev;
    private PlexilNameType next;

    private static int upperBound = 0;
    private static PlexilNameType first = null;
    private static PlexilNameType last = null;

    private PlexilNameType(String anId, String plexName)
    {
	this.id = anId;
        this.prettyName = plexName;
	this.ord = upperBound++;
	if (first == null)
	    {
		first = this;
	    }
	if (last != null)
	    {
		this.prev = last;
		last.next = this;
	    }
	last = this;
    }

    public static Enumeration elements()
    {
	return new Enumeration()
	    {
		private PlexilNameType curr = first;
		public boolean hasMoreElements()
		{
		    return curr != null;
		}
		public Object nextElement()
		{
		    PlexilNameType c = curr;
		    curr = curr.next();
		    return c;
		}
	    } ;
    }

    public String toString() { return this.id; }
    public String name() { return this.id; }
    public String plexilName() { return this.prettyName; }
    public static int size() { return upperBound; }
    public static PlexilNameType first() { return first; }
    public static PlexilNameType last() { return last; }
    public PlexilNameType next() { return this.next; }
    public PlexilNameType prev() { return this.prev; }

    //
    // Members of the enumeration class
    //
    public static final PlexilNameType UNDEFINED_NAME = new PlexilNameType("UNDEFINED_NAME", "_UNDEFINED_");
    public static final PlexilNameType NODE_NAME = new PlexilNameType("NODE_NAME", "Node");
    public static final PlexilNameType VARIABLE_NAME = new PlexilNameType("VARIABLE_NAME", "Variable");
    public static final PlexilNameType FUNCTION_NAME = new PlexilNameType("FUNCTION_NAME", "Function");
    public static final PlexilNameType COMMAND_NAME = new PlexilNameType("COMMAND_NAME", "Command");
    public static final PlexilNameType EXTERNAL_STRUCT_NAME = new PlexilNameType("EXTERNAL_STRUCT_NAME", "_EXTERNAL_STRUCT_");
    public static final PlexilNameType STATE_NAME = new PlexilNameType("STATE_NAME", "State");
    public static final PlexilNameType PARAMETER_NAME = new PlexilNameType("PARAMETER_NAME", "Parameter");
    public static final PlexilNameType LIBRARY_NODE_NAME = new PlexilNameType("LIBRARY_NODE_NAME", "LibraryNode");

}
