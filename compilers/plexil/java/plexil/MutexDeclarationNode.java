// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class MutexDeclarationNode extends PlexilTreeNode
{
    public MutexDeclarationNode(Token t)
    {
        super(t);
    }

    public MutexDeclarationNode(MutexDeclarationNode n)
    {
        super(n);
    }

    //
    // Structure is ^(MUTEX_KWYD NCNAME*)
    //

    @Override
    public void earlyCheck(NodeContext context, CompilerState state)
    {
        if (context.isGlobalContext()) {
            // Top-level declaration
            // Check for duplicate
            for (int i = 0; i < this.getChildCount(); ++i)  {
                PlexilTreeNode n = this.getChild(i);
                String name = n.getText();
                MutexName mn = context.getMutex(name);
                if (mn != null)
                    state.addDiagnostic(n,
                                        "In global declarations: Mutex \"" + name
                                        + "\" is already declared",
                                        Severity.WARNING);
                else
                    context.addMutex(n);
            }
        }
        else {
            // Node level declaration
            for (int i = 0; i < this.getChildCount(); ++i) {
                PlexilTreeNode n = this.getChild(i);
                String name = n.getText();
                // Check for duplicates and shadowing
                MutexName m = context.getMutex(name);
                if (m != null) {
                    // Already declared somewhere, figure out where
                    NodeContext c = m.getContext();
                    if (context == c) {
                        state.addDiagnostic(n,
                                            "In node " + context.getNodeName()
                                            + ": Mutex \"" + name
                                            + "\" is already declared in this node",
                                            Severity.ERROR);
                        continue;
                    }
                    else if (c.isGlobalContext())
                        state.addDiagnostic(n,
                                            "In node " + context.getNodeName()
                                            + ": Mutex \"" + name
                                            + "\" shadows a global mutex of the same name",
                                            Severity.WARNING);
                    else if (c == null)
                        state.addDiagnostic(n,
                                            "Internal error: Mutex \"" + name
                                            + "\" has no context!",
                                            Severity.FATAL);
                    else
                        state.addDiagnostic(n,
                                            "In node " + context.getNodeName()
                                            + ": Mutex \"" + name
                                            + "\" shadows a local mutex declared in node "
                                            + c.getNodeName(),
                                            Severity.WARNING);
                }
                context.addMutex(n);
            }
        }
    }

    // XML generation is delegated to MutexName.
    // See MutexName.makeDeclarationXML().
    @Override
    public Element getXML(Document root)
    {
        return null;
    }

}
