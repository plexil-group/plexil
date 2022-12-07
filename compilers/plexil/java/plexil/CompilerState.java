/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

import java.util.ArrayList;
import java.util.List;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Text;

public class CompilerState
{
    protected CommandParser m_commandParser = null;
    protected File m_infile = null; //* as supplied, may be null
    protected GlobalContext m_globalContext = null;

    protected CharStream m_instream; //* the stream to use initially

    public RecognizerSharedState sharedState; //* shared state to pass between parsers

    List<Diagnostic> m_diagnostics = new ArrayList<Diagnostic>();

    private Document m_rootDocument = null;

    // protected static CompilerState s_instance = null; // FIXME

    public CompilerState(File infile, CommandParser cp)
    {
        m_commandParser = cp;
        m_infile = infile;
        m_globalContext = new GlobalContext();
        sharedState = new RecognizerSharedState();

        // open stream
        if (m_infile == null) {
            // stream is standard input (FIXME)
            try {
                m_instream = new ANTLRInputStream(System.in);
            }
            catch (IOException x) {
                System.err.println("Fatal error opening standard input: " +  x.toString());
                System.exit(1);
            }
        }
        else {
            // open the file
            m_instream = openInputFile(m_infile);
            if (m_instream == null) {
                System.exit(1);
            }
        }
    }

    public String getSourceFileName()
    { 
        return m_infile.toString(); 
    }

    public GlobalContext getGlobalContext()
    {
        return m_globalContext;
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
        if (m_infile == null) {
            return null;
        }
        else {
            return m_commandParser.getOutputFile(m_infile, "epx");
        }
    }

    public File getOutputFile()
    {
        if (m_infile == null) {
            return null;
        }
        else {
            return m_commandParser.getOutputFile(m_infile, "plx");
        }
    }

    public void addDiagnostic(Diagnostic d)
    {
        m_diagnostics.add(d);
    }

    public void addDiagnostic(PlexilTreeNode location, String message, Severity severity)
    {
        m_diagnostics.add(new Diagnostic(m_infile.toString(), location, message, severity));
    }

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

    public void setRootDocument(Document root)
    {
        m_rootDocument = root;
    }

    public Document getRootDocument()
    {
        return m_rootDocument;
    }

}
