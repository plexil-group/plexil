/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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
// import javax.xml.transform.dom.DOMResult; // future
import javax.xml.transform.dom.DOMSource;
// import javax.xml.transform.stream.StreamResult; // future
import javax.xml.transform.stream.StreamSource;

import net.sf.saxon.s9api.Destination;
import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.Serializer;
import net.sf.saxon.s9api.XmlProcessingError;
import net.sf.saxon.s9api.XsltCompiler;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
// import net.sf.saxon.s9api.Xslt30Transformer; // future?

import org.w3c.dom.Document;

public class SaxonTransformer
{
    private Processor m_processor = null;
    private XsltTransformer m_transformer = null;
    private String m_method;
    private boolean m_indent;

    public SaxonTransformer()
    {
        m_processor = new Processor(false);
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
    protected synchronized void loadStylesheet(StreamSource source)
    {
        List<XmlProcessingError> errors = new ArrayList<XmlProcessingError>();
        XsltCompiler compiler = m_processor.newXsltCompiler();
        compiler.setErrorList(errors);
        try {
            XsltExecutable executable = compiler.compile(source);
            m_transformer = executable.load(); // load30()?
        } catch (SaxonApiException e) {
            System.err.println("SaxonTransformer: Error compiling stylesheet:\n" + e.toString());
            // TODO: list errors
        }
    }

    public void loadStylesheetFromFile(File stylesheetFile)
    {
        if (stylesheetFile == null) {
            System.err.println("SaxonTransformer: stylesheet file is null.");
            return;
        }
        if (!stylesheetFile.isFile()) {
            System.err.println("SaxonTransformer: stylesheet file "
                               + stylesheetFile.toString() + " not found.");
            return;
        }
        loadStylesheet(new StreamSource(stylesheetFile));
    }

    public void loadStylesheetFromURL(String urlString)
    {
        if (urlString == null) {
            System.err.println("SaxonTransformer: stylesheet URL is null.");
            return;
        }
        loadStylesheet(new StreamSource(urlString));
    }

    // Construct a serializer to write output of the translator to a file.
    protected Serializer makeFileSerializer(File dest)
    {
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

    protected synchronized boolean translateInternal(Source src, Destination dest)
    {
        if (m_transformer == null) {
            System.err.println("SaxonTransformer: No stylesheet has been loaded\n");
            return false;
        }
        m_transformer.setSource(src);
        m_transformer.setDestination(dest);
        try {
            m_transformer.transform();
        } catch (SaxonApiException s) {
            System.err.println("SaxonTransformer: Unable to perform transformation:\n"
                               + s.toString());
            return false;
        }
        return true;
    }

    public boolean translateDOM(Document n, File out)
    {
        return translateInternal(new DOMSource(n),
                                 makeFileSerializer(out));
    }

    public boolean translateFiles(File in, File out)
    {
        return translateInternal(new StreamSource(in),
                                 makeFileSerializer(out));
    }

}
