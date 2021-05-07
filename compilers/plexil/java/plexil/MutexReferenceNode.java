// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

public class MutexReferenceNode extends PlexilTreeNode
{
    public MutexReferenceNode(Token t)
    {
        super(t);
    }

    public MutexReferenceNode(MutexReferenceNode n)
    {
        super(n);
    }

    //
    // Structure is ^(USING_KYWD NCNAME+)
    //

    public void earlyCheck(NodeContext context, CompilerState state)
    {
        for (int i = 0; i < this.getChildCount(); ++i) {
            PlexilTreeNode n = this.getChild(i);
            String name = n.getText();
            MutexName m = context.getMutex(name);

            // Is it declared?
            if (m == null) {
                state.addDiagnostic(n,
                                    "In node " + context.getNodeName()
                                    + ": No mutex named \"" + name + "\" in current scope",
                                    Severity.ERROR);
            }

            // Is it already in use?
            NodeContext a = context.contextUsing(m);
            if (a != null) {
                if (a == context)
                    state.addDiagnostic(n,
                                        "In node " + context.getNodeName()
                                        + ": Mutex \"" + name
                                        + "\" is already in use in this node",
                                        Severity.ERROR);
                else
                    state.addDiagnostic(n,
                                        "In node " + context.getNodeName()
                                        + ": Mutex \"" + name
                                        + "\" is already in use by ancestor node "
                                        + a.getNodeName(),
                                        Severity.ERROR);
            }

            context.addUsingMutex(m);
        }
    }

}
