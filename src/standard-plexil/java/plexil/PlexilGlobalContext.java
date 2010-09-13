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
import java.util.Map;
import java.util.HashMap;
import java.util.Vector;
import antlr.collections.AST;

import plexil.PlexilNodeContext;
import plexil.PlexilNameType;

//
// A singleton class to represent the global plan environment in the translator.
//

public class PlexilGlobalContext
    extends PlexilNodeContext
{
    protected Map<String, PlexilGlobalDeclaration> commands;
    protected Map<String, PlexilGlobalDeclaration> lookups;
    protected Map<String, PlexilGlobalDeclaration> libraryNodes;

    static PlexilGlobalContext s_instance = null;

    static PlexilGlobalContext getGlobalContext()
    {
	if (s_instance == null)
	    s_instance = new PlexilGlobalContext();
	return s_instance;
    }

    protected PlexilGlobalContext()
    {
	super(null, "_GLOBAL_CONTEXT_");
	commands = new HashMap<String, PlexilGlobalDeclaration>();
	lookups = new HashMap<String, PlexilGlobalDeclaration>();
	libraryNodes = new HashMap<String, PlexilGlobalDeclaration>();
    }

    public boolean isGlobalContext()
    {
	return true;
    }

    protected PlexilGlobalDeclaration getCommand(String name)
    {
	PlexilGlobalDeclaration ln = commands.get(name);
	return ln;
    }

    public boolean isCommandName(String name)
    {
	PlexilGlobalDeclaration ln = getCommand(name);
	return (ln != null);
    }

    public void addCommandName(String name, 
			       AST parm_spec,
			       AST return_spec)
    {
	commands.put(name, 
		     new PlexilGlobalDeclaration(name,
						 PlexilNameType.COMMAND_NAME,
						 parm_spec,
						 return_spec));
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Added command '");
		    plexil.Parse.debugWriter.write(name);
		    plexil.Parse.debugWriter.write("' to global context\n");
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }

    protected PlexilGlobalDeclaration getLookup(String name)
    {
	PlexilGlobalDeclaration ln = lookups.get(name);
	return ln;
    }

    public boolean isLookupName(String name)
    {
	PlexilGlobalDeclaration ln = getLookup(name);
	return (ln != null);
    }

    // *** to do:
    //  - throw semantic exception on redefinition
    //  - throw semantic exception on duplicate parm/return names
    public void addLookupName(String name, 
			      AST parm_spec,
			      AST return_spec)
    {
	lookups.put(name, 
		    new PlexilGlobalDeclaration(name,
						PlexilNameType.STATE_NAME,
						parm_spec,
						return_spec));
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Added lookup '");
		    plexil.Parse.debugWriter.write(name);
		    plexil.Parse.debugWriter.write("' to global context\n");
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }

    protected PlexilGlobalDeclaration getLibraryNode(String name)
    {
	PlexilGlobalDeclaration ln = libraryNodes.get(name);
	return ln;
    }

    public boolean isLibraryNodeName(String name)
    {
	PlexilGlobalDeclaration ln = getLibraryNode(name);
	return (ln != null);
    }

    public void addLibraryNode(String name, AST parm_spec)
    {
	libraryNodes.put(name, 
			 new PlexilGlobalDeclaration(name,
						     PlexilNameType.LIBRARY_NODE_NAME,
						     parm_spec,
						     null));
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Added library node '");
		    plexil.Parse.debugWriter.write(name);
		    plexil.Parse.debugWriter.write("' to global context\n");
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }


    // Caller is responsible for creating the 2 vectors.
    // Any of the arguments may be null, in which case that arg is ignored.
    public void getGlobalDeclarations(Vector<PlexilGlobalDeclaration> commandsResult,
				      Vector<PlexilGlobalDeclaration> lookupsResult)
    {
	if (commandsResult != null)
	    {
		commandsResult.removeAllElements();
		for (Iterator<PlexilGlobalDeclaration> commandIt = commands.values().iterator(); commandIt.hasNext(); )
		    commandsResult.add(commandIt.next());
	    }
	if (lookupsResult != null)
	    {
		lookupsResult.removeAllElements();
		for (Iterator<PlexilGlobalDeclaration> lookupIt = lookups.values().iterator(); lookupIt.hasNext(); )
		    lookupsResult.add(lookupIt.next());
	    }
    }


};
