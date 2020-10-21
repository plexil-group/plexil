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
import javax.xml.transform.stream.StreamSource;

import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.Serializer;
import net.sf.saxon.s9api.StaticError;
import net.sf.saxon.s9api.XsltCompiler;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
// import net.sf.saxon.s9api.Xslt30Transformer; // future?


public class SaxonTransformer
{
    private Processor m_processor;
    private XsltCompiler m_compiler;

    public SaxonTransformer()
    {
        // TODO: configuration
        m_processor = new Processor(false);
        m_compiler = m_processor.newXsltCompiler();
    }

    // Construct an XsltExecutable from the given stylesheet path.
    protected XsltExecutable makeExecutable(File stylesheet)
    {
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

    // Export the precompiled stylesheet to the given file.
    protected static boolean exportCompiledStylesheet(XsltExecutable exe, File dest)
    {
        if (exe == null)
            return false;
        try (OutputStream out = new FileOutputStream(dest);) {
            exe.export(out);
        } catch (FileNotFoundException f) {
            System.err.println("SaxonTransformer: Unable to open "
                               + dest.toString() + " for output:\n" + f.toString());
            return false;
        } catch (SecurityException s) {
            System.err.println("SaxonTransformer: Unable to open "
                               + dest.toString() + " for output:\n" + s.toString());
            return false;
        } catch (SaxonApiException e) {
            System.err.println("SaxonTransformer: Unable to export compiled stylesheet to "
                               + dest.toString() + ":\n" + e.toString());
            return false;
        } catch (IOException i) {
            System.err.println("SaxonTransformer: IO error while exporting compiled stylesheet to "
                               + dest.toString() + ":\n" + i.toString());
            return false;
        }
        return true;
    }

    static protected File exportFilename(File sourceFilename)
    {
        return new File(sourceFilename.getParentFile(),
                        sourceFilename.getName() + ".sef");
    }

    // Load a stylesheet. Load the compiled version if available.n
    // Otherwise, load the source and attempt to save out the compiled version.
    // Returns an XsltExecutable on success, null on failure to load.
    // (Errors during save are reported but do not cause the function to fail.)
    protected XsltExecutable loadStylesheet(File stylesheet)
    {
        if (!stylesheet.isFile()) {
            System.err.println("Error: stylesheet file " + stylesheet.toString()
                               + " not found.");
            return null;
        }

        // Check if the precompiled file exists and is newer.
        XsltExecutable result = null;
        File compiledPath = exportFilename(stylesheet);
        if (compiledPath.isFile() &&
            compiledPath.lastModified() > stylesheet.lastModified()) {
            result = makeExecutable(compiledPath);
            if (result != null)
                return result;
        }

        // Load it and attempt to compile 
        result = makeExecutable(stylesheet);
        if (result == null)
            return null;

        // Try to export it to the same directory.
        // We don't care if it doesn't succeed.
        exportCompiledStylesheet(result, compiledPath);

        return result;
    }

    // Construct a serializer to write output of the translator to a file.
    protected Serializer makeFileSerializer(File dest)
    {
        if (m_processor == null)
            return null;
        Serializer result = m_processor.newSerializer(dest);
        result.setOutputProperty(Serializer.Property.METHOD, "xml");
        return result;
    }

    // Construct a serializer to write output of the translator to a file.
    protected Serializer makeFileSerializer(File dest, boolean indent)
    {
        Serializer result = makeFileSerializer(dest);
        if (result == null)
            return null;
        result.setOutputProperty(Serializer.Property.INDENT, (indent ? "yes" : "no"));
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

    public boolean translateFiles(File ss, File in, File out)
    {
        XsltExecutable exe = loadStylesheet(ss);
        if (exe == null)
            return false;

        if (!in.isFile()) {
            System.err.println("Error: XSLT input file " + in.toString() + " not found");
            return false;
        }
        StreamSource src = new StreamSource(in);

        Serializer ser = makeFileSerializer(out);

        return translateInternal(exe.load(), src, ser);
    }

}
