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

package plexil.xml;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.util.ArrayList;
import java.util.List;

import javax.xml.transform.Source;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;

import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.Serializer;
import net.sf.saxon.s9api.StaticError;
import net.sf.saxon.s9api.XsltCompiler;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
// import net.sf.saxon.s9api.Xslt30Transformer; // future?

import org.w3c.dom.Document;

public class SaxonTransformer
{
    private Processor m_processor;
    private XsltCompiler m_compiler;
    private String m_method;
    private boolean m_indent;

    public SaxonTransformer()
    {
        m_processor = new Processor(false);
        m_compiler = m_processor.newXsltCompiler();
        m_method = "xml"; 
        m_indent = false;
    }

    public boolean getIndent()
    {
        return m_indent;
    }

    public void setIndent(boolean indent)
    {
        m_indent = indent;
    }

    public String getMethod()
    {
        return m_method;
    }

    public void setMethod(String outputMethod)
    {
        m_method = outputMethod;
    }

    // Construct an XsltExecutable from the given stylesheet path.
    protected XsltExecutable loadStylesheet(File stylesheet)
    {
        if (!stylesheet.isFile()) {
            System.err.println("Error: stylesheet file " + stylesheet.toString()
                               + " not found.");
            return null;
        }

        List<StaticError> errors = new ArrayList<StaticError>();
        m_compiler.setErrorList(errors);
        try {
            return m_compiler.compile(new StreamSource(stylesheet));
        } catch (SaxonApiException e) {
            System.err.println("SaxonTransformer: Error compiling stylesheet "
                               + stylesheet.toString() + " :\n" + e.toString());
            // TODO: list errors
            return null;
        }
    }

    // Construct a serializer to write output of the translator to a file.
    protected Serializer makeFileSerializer(File dest)
    {
        if (m_processor == null)
            return null;
        Serializer result = m_processor.newSerializer(dest);
        try {
            result.setOutputProperty(Serializer.Property.METHOD, m_method);
            result.setOutputProperty(Serializer.Property.INDENT,
                                     m_indent ? "yes" : "no");
        } catch (IllegalArgumentException i) {
            System.err.println("SaxonTransformer: error setting output properties:\n"
                               + i.toString());
        }
        return result;
    }

    protected static boolean translateInternal(XsltTransformer trans, Source src, Serializer dest)
    {
        trans.setSource(src);
        trans.setDestination(dest);
        try {
            trans.transform();
        } catch (SaxonApiException s) {
            System.err.println("SaxonTransformer: Unable to perform transformation:\n"
                               + s.toString());
            return false;
        }
        return true;
    }

    public boolean translateDOM(File ss, Document n, File out)
    {
        XsltExecutable exe = loadStylesheet(ss);
        if (exe == null)
            return false;
        return translateInternal(exe.load(),
                                 new DOMSource(n),
                                 makeFileSerializer(out));
    }

    public boolean translateFiles(File ss, File in, File out)
    {
        XsltExecutable exe = loadStylesheet(ss);
        if (exe == null)
            return false;

        if (!in.isFile()) {
            System.err.println("Error: XSLT input file " + in.toString() + " not found");
            return false;
        }
        return translateInternal(exe.load(),
                                 new StreamSource(in),
                                 makeFileSerializer(out));
    }

}
