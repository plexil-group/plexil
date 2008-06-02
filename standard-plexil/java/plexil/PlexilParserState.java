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

import java.io.File;
import java.util.Stack;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Collections;
import antlr.RecognitionException;
import antlr.CommonAST;
import antlr.Token;
import antlr.collections.AST;
import antlr.SemanticException;

//
// *** TO DO:
//  - Parser input may not be from a "file"
//

/*
	A class to maintain the state of a PlexilParser.  
        This is also a mechanism to pass information out
	to the tree parser.
*/
public class PlexilParserState implements PlexilTokenTypes
{

    private PlexilNodeContext context = null;
    private int anonymousContext = 0;
    protected int errors = 0;
    protected int warnings = 0;
    protected Map nodeNames = new HashMap();
    protected Map symbols = new HashMap();
    protected Stack file = new Stack();

    public PlexilParserState(PlexilParserState state)
    {
	this();
	putAll(state);
    }

    public void pushFile(File file)
    {
	this.file.push(file);
    }

    public File popFile()
    {
	return (File)file.pop();
    }

    public File getFile()
    {
	return (File)file.peek();
    }

    public PlexilParserState()
    {
    }

    public void putAll(PlexilParserState state)
    {
	nodeNames.putAll(state.nodeNames);
	symbols.putAll(state.symbols);
	file.addAll(state.file);
	errors += state.errors;
	warnings += state.warnings;
    }

    public void warn(String message)
    {
	++warnings;
	System.err.println(getErrorPrefix(null)+message);
    }

    public void warn(Exception ex)
    {
	++warnings;
	System.err.println(getErrorPrefix(ex)+ex.getMessage());
    }

    public void error(Exception ex)
    {
	++errors;
	System.err.println(getErrorPrefix(ex)+ex.getMessage());
    }

    private String getErrorPrefix(Exception ex)
    {
	StringBuffer prefix = new StringBuffer(24);
	if (ex instanceof RecognitionException)
	    {
		RecognitionException re = (RecognitionException)ex;
		if (re.getFilename()!=null)
		    prefix.append(re.getFilename());
		else
		    prefix.append(getFile().getName());
		if (re.getLine()!=0)
		    {
			prefix.append(":").append(re.getLine());
			if (re.getColumn()!=0)
			    prefix.append(":").append(re.getColumn());
		    }
	    }
	else
	    {
		prefix.append(getFile().getName());
	    }
	return prefix.append(": ").toString();
    }

    public boolean printErrorCount()
    {
	if (warnings == 1) System.err.println("1 warning");
	else if (warnings > 1) System.err.println(warnings+" warnings");
	if (errors == 0) return false;
	else if (errors == 1) System.err.println("1 error");
	else System.err.println(errors+" errors");
	return true;
    }

    public PlexilNodeContext getContext()
    {
	return context;
    }

    public String toString()
    {
	StringBuffer toRet = new StringBuffer(2000);
	toRet.append("Context: ").append(context).append("\n");
	toRet.append("Anonymous Context: ").append(anonymousContext).append("\n");
	// sort and print the names (values of the nodeNames map) by name alpha
	toRet.append("Node Names: \n");
	List sortedNames = new ArrayList(nodeNames.keySet()); 
	Collections.sort(sortedNames);
	for (int i = 0; i < sortedNames.size(); i++)
	    toRet.append("  ").append(nodeNames.get(sortedNames.get(i))).append("\n");
	return toRet.toString();
    }

    // lispy ways of getting substrings from a context
    protected static String first(String s)
    {
	if (s == null) return "";
	int firstDot = s.indexOf('.');
	if (firstDot==-1) return s;
	else             return s.substring(0,firstDot);
    }

    protected static String rest(String s)
    {
	if (s == null) return null;
	int firstDot = s.indexOf('.');
	if (firstDot==-1) return null;
	else             return s.substring(firstDot+1);
    }

    protected static String last(String s)
    {
	if (s == null) return "";
	int lastDot = s.lastIndexOf('.');
	if (lastDot==-1) return s;
	else            return s.substring(lastDot+1);
    }

    protected static String butLast(String s)
    {
	if (s == null) return null;
	int lastDot = s.lastIndexOf('.');
	if (lastDot==-1) return null;
	else            return s.substring(0,lastDot);
    }

    protected static String qualify(String context, String name)
    {
	if (context == null) return name;
	if (name == null) return context;
	if (context.length()>0&&name.length()>0)
	    return context+"."+name;
	else if (context.length()>0)
	    return context;
	return name;
    }

    public void addNode(String name, PlexilNodeContext c)
    {
	context = c;
	if (name != null)
	    {
		nodeNames.put(name, c);
                // System.out.println("addNode(" + name + ")");
	    }
    }

    public boolean isNodeName(String name)
    {
	PlexilNodeContext c = (PlexilNodeContext) nodeNames.get(name);
        boolean result = (c != null);
//         System.out.println("isNodeName(" + name + ") returns " +
//                            (result ? "true" : "false"));
	return result;
    }

}
