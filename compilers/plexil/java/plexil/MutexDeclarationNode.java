// Copyright (c) 2006-2016, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package plexil;

import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.XMLElement;
import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

public class MutexDeclarationNode extends PlexilTreeNode
{
    boolean m_isGlobal = false;

    public MutexDeclarationNode(Token t)
    {
        super(t);
    }

    public MutexDeclarationNode(MutexDeclarationNode n)
    {
        super(n);
    }

    //
    // Structure is ^(MUTEX_KWYD NCNAME)
    //

    public String getMutexName()
    {
        return this.getChild(0).getText();
    }

    public void earlyCheck(NodeContext context, CompilerState state)
    {
        if (context.isGlobalContext()) {
            // Top-level declaration
            m_isGlobal = true;

            // Only one child allowed
            if (this.getChild(1) != null) {
                state.addDiagnostic(this.getChild(1),
                                    "Mutex declarations may only name one mutex per declaration",
                                    Severity.ERROR);
            }

            // Check for duplicate
            String mutexName = getMutexName();
            if (context.isMutexName(mutexName)) {
                state.addDiagnostic(this.getChild(0),
                                    "Mutex \"" + mutexName + "\" is already declared in this plan",
                                    Severity.WARNING);
            }
            context.addMutex(mutexName);
        }
        else {
            // Node level declaration
            for (int i = 0; i < this.getChildCount(); ++i) {
                PlexilTreeNode n = this.getChild(i);
                String mutexName = n.getText();
                // Check for top-level declaration
                if (!GlobalContext.getGlobalContext().isMutexName(mutexName)) {
                    state.addDiagnostic(n,
                                        "Mutex \"" + mutexName + "\" lacks a global declaration",
                                        Severity.ERROR);
                }

                // Check for duplicate in node
                if (context.isMutexName(mutexName)) {
                    state.addDiagnostic(n,
                                        "Mutex \"" + mutexName + "\" is already held in this node",
                                        Severity.ERROR);
                }

                // Check for duplicate in ancestors
                if (context.isMutexInherited(mutexName)) {
                    state.addDiagnostic(n,
                                        "Mutex \"" + mutexName + "\" is already held by an ancestor node",
                                        Severity.ERROR);
                }

                context.addMutex(mutexName);
            }
        }
    }

    public void constructXML()
    {
        if (m_isGlobal) {
            IXMLElement nameXML = new XMLElement("Name");
            super.constructXML();
            nameXML.setContent(this.getChild(0).getText());
            m_xml.addChild(nameXML);
        }
    }

    public String getXMLElementName()
    {
        if (m_isGlobal)
            return "MutexDeclaration";
        else
            return "Mutexes";
    }

}
