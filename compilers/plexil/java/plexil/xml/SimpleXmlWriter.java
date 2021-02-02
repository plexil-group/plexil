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
import java.io.OutputStream;

import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;

import javax.xml.transform.dom.DOMSource;

import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Node;

public class SimpleXmlWriter
{
    private static TransformerFactory s_factory = null;

    private Transformer m_xformer;
    private Result m_result;

    public SimpleXmlWriter(OutputStream s)
    {
        try {
            m_xformer = getTransformerFactory().newTransformer();
        } catch (TransformerConfigurationException c) {
            System.err.println("Error creating XML writer:\n"
                               + c.toString());
        }
        m_xformer.setOutputProperty(OutputKeys.METHOD, "xml");
        m_result = new StreamResult(s);
    }

    public void setIndent(boolean indent)
    {
        try {
            m_xformer.setOutputProperty(OutputKeys.INDENT,
                                        (indent ? "yes" : "no"));
        } catch (IllegalArgumentException i) {
            System.err.println("Error: unable to set indent property:\n"
                               + i.toString());
        }
    }

    public void write(Node domNode)
    {
        Source src = new DOMSource(domNode);
        try {
            m_xformer.transform(src, m_result);
        } catch (TransformerException t) {
            System.err.println("Error writing XML output:\n"
                               + t.toString());
        }
    }

    private static TransformerFactory getTransformerFactory()
    {
        if (s_factory == null) {
            s_factory = TransformerFactory.newInstance();
        }
        return s_factory;
    }
    
}
