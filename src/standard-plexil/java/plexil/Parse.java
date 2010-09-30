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

import java.util.*;
import java.io.*;
import net.n3.nanoxml.*;

/**
 * Creation date: (1/26/2004 12:07:04 PM)
 * @author: Andrew Bachmann
 */
public class Parse
{
    public static final String version = new String("PlexilParser version 0.4");

    public static boolean quiet = false;
    public static FileWriter debugWriter = null;

    public Parse() 
    {
        super();
    }
    
    private static void exit(int value) 
    {
	if (value != 0) 
	    {
		System.out.println("PlexilParser exiting.");
	    }
        System.exit(value);
    }

    private static File outputFilename(String inFileName, String outFileName, boolean isExtendedPlexil)
    {
		File infile = new File(inFileName);

		if (outFileName == null) {
			// Replace the infile's extension with .epx or .plx as required.
			String outfile_basis = infile.getName();
			int index = outfile_basis.lastIndexOf('.');
			String outname = 
				outfile_basis.substring(0, index != -1 ? index : outfile_basis.length()) +
				(isExtendedPlexil ? ".epx" : ".plx");
			return new File(infile.getParent(), outname);
		}
		else if (isExtendedPlexil) {
			// Use the specified output file, but replace the extension with .epx.
			int index = outFileName.lastIndexOf('.');
			String outname = 
				outFileName.substring(0, index != -1 ? index : outFileName.length()) +
				(isExtendedPlexil ? ".epx" : ".plx");
			return new File(outname);
		}
		else {
			// The user specified an output file, so use it.
			return new File(outFileName);
		}
    }
    
    private static void printUsage() 
    {
        System.out.println(version);
        System.out.println("Usage: plexil [OPTION]... <source file>");
        System.out.println("");
        System.out.println("  -d, --debug <filename>     Print debug information to filename");
        System.out.println("  -h, --help                 Print this help and exit");
        System.out.println("  -o, --output <filename>    Write translator output to filename");
        System.out.println("  -q, --quiet                Parse files quietly");
        System.out.println("  -v, --version              Print the translator version and exit");
        System.out.println("");
        System.out.println("Example:");
        System.out.println("plexil -o filename.plx filename.ple");
    }
  
    private static Vector<String> getArgs(String[] args) 
    {
        Vector<String> arguments = new Vector<String>();
        for (int i = 0 ; i < args.length ; i++) 
	    {
		if (!args[i].equals("-a") && !args[i].equals("--arguments")) 
		    {
			arguments.add(args[i]);
		    } 
		else
		    {
			i += 1;
			if (i == args.length) 
			    {
				System.out.println("-a or --arguments requires a filename");
				printUsage();
				exit(-1);
			    }
			try 
			    {
				File file = new File(args[i]);
				FileReader fis = new FileReader(file);
				BufferedReader buf = new BufferedReader(fis);
				while (true) 
				    {
					String line = buf.readLine();
					if (line == null) 
					    {
						break;
					    }
					arguments.add(line.trim());
				    }
			    } 
			catch (Exception e) 
			    {
				System.out.println(e);
				printUsage();
				exit(-1);
			    }
		    }
	    }
        return arguments;
    }

    public static final File generateIncludeFileName(final String parent, final String filename) 
    {
	File returnFile = new File("");
	// handle the case of the parent dir being empty. 
	if (parent != "")
	    {
		returnFile = new File(parent, filename);
	    }
	else
	    {
		returnFile = new File(filename);
	    }
	if (returnFile.canRead()) 
	    {
		// file exists, no need to search the search path. 
		return returnFile;
	    } 
	else 
	    {
		// search for file on search path. 
		// retrive search path.
		//Object[] customSearchPath = ModelAccessor.getCustomPath();
		Object[] customSearchPath = new Object[0];
		// if search path exists.
		if (customSearchPath.length > 0 ) 
		    {
			//System.out.print("File " + returnFile.toString() + " was not found. ");
			//System.out.println("Searching for it in the nddl_path... ");
			// create a version of the filename with all path info removed.
			String filenameWithoutPath = "";
			int lastDirSlash = filename.lastIndexOf('/');
			if (lastDirSlash != -1) 
			    {
				filenameWithoutPath = 
				    filename.substring(lastDirSlash + 1, filename.length());
			    }
			File candidateFile = new File("");
			boolean found = false;
			for (int i = 0; i < customSearchPath.length; i++) 
			    {
				String pathEntry = (String) customSearchPath[i];
				// check if file exists with path info from include statement. 
				candidateFile = new File(pathEntry, filename);
				//System.out.println("checking for " + candidateFile);
				if (candidateFile.canRead()) 
				    {
					found = true;
					break;
				    }
				// check if file exists without path info from include statement.
				candidateFile = new File(pathEntry, filenameWithoutPath);
				// System.out.println("checking for " + candidateFile);
				if (candidateFile.canRead()) {
				    found = true;
				    break;
				}
			    } // end examine each path. 
			if (found) 
			    {
				//System.out.println("File " + candidateFile.toString() + " found in search path.");
				return candidateFile;
			    }
			else 
			    {
				//System.out.println("The file was not found in the nddl_path.");
			    }
		    }
	    }
	return returnFile; // default behaviour - let caller handle case when the file is not found 
    }

    public static boolean load(IXMLElement xml, String filename)
	throws Exception 
    {
	if (!quiet) 
	    {
		System.out.println("  " + filename);
	    }
	File file = generateIncludeFileName("", filename); 
        if (!file.canRead()) 
	    {
		throw new FileNotFoundException("unable to read file: " + filename);
	    }
	PlexilParser parser = PlexilParser.parse(file, debugWriter);
	if (parser.getState().printErrorCount()) 
	    {
		// be sure to properly close debug writer if bailing out early
		if (debugWriter != null)
		    debugWriter.close();
		System.exit(1);
	    }
	if (debugWriter != null)
	    {
		antlr.collections.AST parseAST = parser.getAST();
		if (parseAST != null)
		    {
			debugWriter.write(parseAST.toStringTree());
		    }
		else
		    {
			debugWriter.write("Parser AST is null!");
		    }
		debugWriter.write('\n');
	    }

       	PlexilTreeParser treeParser = new PlexilTreeParser(parser.getState());
       	treeParser.plexilPlan(parser.getAST(), xml);
	if (treeParser.getState().printErrorCount()) 
	    {
		// be sure to properly close debug writer if bailing out early
		if (debugWriter != null)
		    debugWriter.close();
		System.exit(1);
	    }
	return treeParser.getState().isExtendedPlexil();
    }
    
    public static void main(String [] args) throws Exception 
    {
        try
	    {
		Vector<String> arguments = getArgs(args);
		if (arguments.isEmpty()) 
		    {
			printUsage();
			exit(0);
		    }
                File outputFile = null;
                String output = null;
		Iterator<String> itr = arguments.iterator();
		boolean filename = false;
		String argument = null;
		while (itr.hasNext()) 
		    {
                        // NOTE: when loop terminates normally, 'argument' is the input filename
			argument = itr.next();
			if (argument.length() == 0) continue;
			if (argument.charAt(0) != '-') 
			    {
				filename = true;
				break;
			    }
			if (argument.equals("-d") || argument.equals("--debug")) 
			    {
				debugWriter = new FileWriter((String)itr.next());
				continue;
			    }
			if (argument.equals("-h") || argument.equals("--help")) 
			    {
				printUsage();
				exit(0);
			    }
			if (argument.equals("-o") || argument.equals("--output")) 
			    {
                                output = (String) itr.next();
				continue;
			    }
			if (argument.equals("-q") || argument.equals("--quiet")) 
			    {
				quiet = true;
				continue;
			    }
			if (argument.equals("-v") || argument.equals("--version")) 
			    {
				System.out.println(version);
				System.exit(0);
			    }
			System.out.println("Error: Unknown flag: "+argument);
			printUsage();
			exit(-1);
		    }
		if (!filename) 
		    {
			System.out.println("Error: no source file name supplied.");
			printUsage();
			exit(-1);
		    }

		if (!quiet) 
		    {
			System.out.println(version);
			System.out.println();
			System.out.println("Translating:");
		    }

		// read in plan and translate
		IXMLElement xml = new XMLElement("PlexilPlan");
		boolean isExtended = load(xml,argument);

		// write out XML
                outputFile = outputFilename(argument, output, isExtended);
		FileWriter writer = new FileWriter(outputFile);
		XMLWriter xmlWriter = new XMLWriter(writer);
		checkXML(xml);
		if (!quiet)
		    {
			System.out.println("Writing "
					   + (isExtended ? "Extended" : "Core")
					   + " PLEXIL to "
					   + outputFile);
		    }
		xmlWriter.write(xml,true,2,true);
		//xmlWriter.write(xml,true,0,true);
		writer.close();

		if (isExtended)
		    {
			// Translate from Extended PLEXIL to Core PLEXIL
			File coreOutput = outputFilename(argument, output, false);
			if (!quiet)
			    {
				System.out.println("Translating to Core PLEXIL file " + coreOutput);
			    }
			String[] saxonArgs = new String[4];
			saxonArgs[0] = "-o";
			saxonArgs[1] = coreOutput.toString();
			saxonArgs[2] = outputFile.toString();
			saxonArgs[3] = System.getenv("PLEXIL_HOME") + "/schema/translate-plexil.xsl";
			net.sf.saxon.Transform.main(saxonArgs);
		    }
		if (!quiet) System.out.println("Done.");
		if (debugWriter != null) 
		    debugWriter.close();
	    } 
	catch (Exception e) 
	    {
		System.err.println();
		e.printStackTrace();
		if (debugWriter != null) 
		    debugWriter.close();
		exit(-1);
	    }
        exit(0);
    }
   
    static void checkXML(IXMLElement xml)
    {
	//System.err.println(xml.getName());
	//xml.getAttributes().list(System.err);
	for (int i=0; i < xml.getChildrenCount(); i++)
	    checkXML(xml.getChildAtIndex(i));
    }
}
