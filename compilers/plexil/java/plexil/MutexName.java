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

package plexil;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class MutexName extends PlexilName
{
    NodeContext m_context;

    public MutexName(PlexilTreeNode decl, NodeContext c)
    {
        super(decl.getText(), NameType.MUTEX_NAME, decl);
        m_context = c;
    }

    public NodeContext getContext()
    {
        return m_context;
    }

    public Element makeDeclarationXML(Document root)
    {
        Element nameElt = root.createElement("Name");
        nameElt.appendChild(root.createTextNode(getName()));
        Element result = root.createElement("DeclareMutex");
        result.appendChild(nameElt);
        if (m_declaration != null) {
            result.setAttribute("LineNo",
                                String.valueOf(m_declaration.getLine()));
            result.setAttribute("ColNo",
                                String.valueOf(m_declaration.getCharPositionInLine()));
        }
        return result;
    }

    // TODO: add null/empty string checks
    public Element asReference(Document root)
    {
        Element result = root.createElement("Name");
        result.appendChild(root.createTextNode(getName()));
        return result;
    }
}
