/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

import org.antlr.runtime.CharStream;
import org.antlr.runtime.ANTLRInputStream;
import org.antlr.runtime.ANTLRFileStream;
import org.antlr.runtime.RecognizerSharedState;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Text;

public class CompilerState
{
    String[] m_args; //* arguments passed to Compiler.main()

    protected String m_sourceFileName = null;
    protected File m_infile = null; //* as supplied, may be null
    protected File m_outfile = null;

    // Processing-related flags
    public boolean debug = false;
    public boolean epxOnly = false;
    public boolean indentOutput = false;
    public boolean semanticsOnly = false;
    public boolean syntaxOnly = false;
    public boolean writeEpx = false;

    protected CharStream m_instream; //* the stream to use initially

    public RecognizerSharedState sharedState; //* shared state to pass between parsers

    Vector<Diagnostic> m_diagnostics = new Vector<Diagnostic>();

    private Document m_rootDocument = null;

    private static DocumentBuilder s_documentBuilder = null;

    protected static CompilerState s_instance = null;

    public CompilerState(String[] args)
    {
        s_instance = this;
        m_args = args;

        // parse args
        if (!parseArgs()) {
            usage();
            System.exit(-1);
        }

        // open stream
        if (m_infile == null) {
            // stream is standard input
            try {
                m_instream = new ANTLRInputStream(System.in);
            }
            catch (IOException x) {
                System.out.println("Fatal error opening standard input: " +  x.toString());
                System.exit(-1);
            }
        }
        else {
            // open the file
            m_instream = openInputFile(m_infile);
            if (m_instream == null) {
                System.exit(-1);
            }
        }
    }

    public String getSourceFileName()
    { 
        return m_sourceFileName; 
    }

    // Singleton accessor is required because some methods which
    // don't get the state passed into them need to generate diagnostics.
    public static CompilerState getCompilerState() { return s_instance; }

    public boolean parseArgs()
    {
        int i = 0; // index of arg being processed

        // Parse options
        while (i < m_args.length) {
            if (m_args[i].equals("-h") || m_args[i].equals("--help")) {
                // Print usage message and exit
                usage();
                System.exit(0);
            }
            else if (m_args[i].equals("-v") || m_args[i].equals("--version")) {
                // Print version and exit
                System.out.println("PlexilCompiler version 2.0.0d1");
                System.exit(0);
            }
            else if (m_args[i].equals("-d") || m_args[i].equals("--debug")) {
                debug = true;
            }
            else if (m_args[i].equals("-e") || m_args[i].equals("--epx-only")) {
                epxOnly = true;
                writeEpx = true;
            }
            else if (m_args[i].equals("-w") || m_args[i].equals("--write-epx")) {
                writeEpx = true;
            }
            else if (m_args[i].equals("-o")) {
                m_outfile = new File(m_args[++i]);
            }
            else if (m_args[i].equals("-p") || m_args[i].equals("--pretty-print")) {
                indentOutput = true;
            }
            else if (m_args[i].equals("-m") || m_args[i].equals("--semantics-only")) {
                semanticsOnly = true;
            }
            else if (m_args[i].equals("-s") || m_args[i].equals("--syntax-only")) {
                syntaxOnly = true;
            }
            else {
                // Not a recognized option, go on to process input file name(s)
                break;
            }
            ++i;
        }

		if (i >= m_args.length) {
			// No file name supplied
			usage();
			System.exit(1);
		}

        // whatever's left must be the source file name(s)
        m_sourceFileName = m_args[i];
        m_infile = new File(m_sourceFileName);
        return true;
    }

    public void usage()
    {
        System.out.println("Usage:  PlexilCompiler [options] [sourcefile]");
        System.out.println(" Options for information (no sourcefile required): ");
        System.out.println("  -h, --help            Prints this message and exits");
        System.out.println("  -v, --version         Prints version number and exits");
        System.out.println(" Options for output control: ");
        System.out.println("  -o <filename>         Writes output to filename");
        System.out.println("  -p, --pretty-print    Format XML output for readability");
        System.out.println(" Options primarily for compiler testing:");
        System.out.println("  -d, --debug           Enable debug output to standard-error stream");
        System.out.println("  -w, --write-epx       Write an Extended Plexil XML output file");
        System.out.println("  -e, --epx-only        Write EPX and do not translate to Core Plexil");
        System.out.println("  -m, --semantics-only  Perform syntax and semantic checks, but do not generate code");
        System.out.println("  -s, --syntax-only     Perform surface syntax parsing only");
    }

    public CharStream openInputFile(File f)
    {
        try {
            return new ANTLRFileStream(f.toString());
        }
        catch (IOException x) {
            System.err.println("Unable to open input file " + f.toString() + ": " + x.toString());
        }
        return null;
    }

    public boolean getDebug() { return debug; }

    public CharStream getInputStream() 
    {
        return m_instream; 
    }

    public OutputStream getEpxStream()
    {
        File epxFile = getEpxFile();
        if (epxFile == null)
            return System.out;

        try {
            return new FileOutputStream(epxFile);
        }
        catch (IOException x) {
            System.err.println("Unable to open Extended Plexil output file "
                               + epxFile.toString() + ": " + x.toString());
            return null;
        }
    }

    public File getEpxFile()
    {
        if (m_outfile == null) {
            if (m_infile == null)
                return null;
            else
                // substitute ".epx" for ".ple" in infile name
                return replaceFileExtension(m_infile, "epx");
        }
        else if (epxOnly)
            return m_outfile;
        else
            // substitute ".epx" for ".plx" in supplied outfile name
            return replaceFileExtension(m_outfile, "epx");
    }

    public File getOutputFile()
    {
        if (m_outfile == null) {
            if (m_infile == null)
                return null;
            else 
                // substitute ".plx" for ".ple" in infile name
                return replaceFileExtension(m_infile, "plx");
        }
        else
            // return spec'd outfile
            return m_outfile;
    }

    private File replaceFileExtension(File f, String newExtension)
    {
        String fname = f.getName();
        int dotIdx = fname.lastIndexOf('.');
        if (dotIdx == -1)
            return new File(f.getParent(),
                            fname + "." + newExtension);
        else
            return new File(f.getParent(),
                            fname.substring(0, dotIdx + 1) + newExtension);
    }

    public void addDiagnostic(Diagnostic d)
    {
        m_diagnostics.add(d);
    }

    public void addDiagnostic(PlexilTreeNode location, String message, Severity severity)
    {
        m_diagnostics.add(new Diagnostic(location, message, severity));
    }

    public Vector<Diagnostic> getDiagnostics() { return m_diagnostics; }

    public void displayDiagnostics()
    {
        for (Diagnostic d : m_diagnostics)
            System.err.println(d.toString());
    }

    public int maxErrorSeverity()
    {
        int result = -1;
        for (Diagnostic d : m_diagnostics) {
            if (d.severity() > result)
                result = d.severity();
        }
        return result;
    }

    public Document getRootDocument()
    {
        // Bootstrapping
        if (s_documentBuilder == null) {
            // Configuration??
            try {
            s_documentBuilder =
                DocumentBuilderFactory.newInstance().newDocumentBuilder();
            } catch (ParserConfigurationException p) {
                System.err.println("Fatal error: unable to boostrap XML construction:\n"
                                   + p.toString());
                return null;
            };
        }

        // Construct a new Extended PLEXIL XML document
        if (m_rootDocument == null) {
            m_rootDocument = s_documentBuilder.newDocument();
            m_rootDocument.setXmlVersion("1.0");
        }
        return m_rootDocument;
    }

    public static Element newElement(String tagName)
    {
        return getCompilerState().getRootDocument().createElement(tagName);
    }

    public static Text newTextNode(String contents)
    {
        return getCompilerState().getRootDocument().createTextNode(contents);
    }

}
