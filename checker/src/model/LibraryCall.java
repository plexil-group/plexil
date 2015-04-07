/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

package model;

import java.util.Vector;

import main.Log;
import model.expr.Expr;
import model.expr.ExprList;
import model.expr.ExprType;

public class LibraryCall
    extends Action {

    public class Alias {
        private String varName;
        private Expr exp;

        public Alias(String name, Expr e) {
            varName = name;
            exp = e;
        }

        public String getVarName() {
            return varName;
        }

        public Expr getExpr() {
            return exp;
        }

        public String toString() {
            return varName + " = " + exp.toString();
        }

        public void check(Node node, GlobalDeclList decls, Vector<Log> errors) {
            // TODO
        }
    }

    private String name;
    private GlobalDecl decl;
    private Vector<Alias> aliases;

    public LibraryCall(String nodeName, GlobalDecl dec) {
        super(ActionType.LibraryCall);
        name = nodeName;
        decl = dec;
        aliases = null;
    }

    public String getName() {
        return name;
    }

    public GlobalDecl getDecl() {
        return decl;
    }

    public Vector<Alias> getAliases() {
        return aliases;
    }

    public void addAlias(String name, Expr e) {
        if (aliases == null)
            aliases = new Vector<Alias>();
        aliases.add(new Alias(name, e));
    }

    public void check(Node node, GlobalDeclList decls, Vector<Log> errors) {
        // TODO
        // check that name is a declared library node (could do at parse time)
        // check alias names
        // check alias expressions
        // check alias type consistency
        // check alias in/out consistency
    }

    @Override
    public String toString() {
        StringBuilder s = new StringBuilder();
        s.append("LibraryCall ");
        s.append(name);
        if (aliases != null) {
            s.append("(");
            s.append(Integer.toString(aliases.size()));
            s.append(" aliases)");
        }
        return s.toString();
    }
}
