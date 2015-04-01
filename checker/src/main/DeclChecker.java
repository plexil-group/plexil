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

package main;

import model.*;
import model.GlobalDecl.CallType;
import model.Action.ActionType;
import model.expr.*;
import model.expr.Expr.*;
import java.util.Vector;

public class DeclChecker {
    
    public Vector<Log> checkPlan(Plan p)
    {
        Vector<Log> errors = new Vector<Log>();
        
        errors.addAll(checkDeclRedundency(p.getDecls()));
        errors.addAll(checkNodeDeclConsistency(p));
        errors.addAll(checkVarAccessibility(p));

        return errors;
    }

    public Vector<Log> checkDeclRedundency(GlobalDeclList decls)
    {
        int i, j;
        Vector<Log> errors = new Vector<Log>();
        
        String[] ids = new String[decls.size()];
        for (i = 0; i < decls.size(); i++)
            {
                ids[i] = decls.elementAt(i).getID();
            }
        
        for (i = 0; i < ids.length - 1; i++)
            {
                for (j = i + 1; j < ids.length; j++)
                    {
                        if (ids[i].equals(ids[j]))
                            errors.add(nameConflictMessage(decls.elementAt(i), decls.elementAt(j)));
                    }
            }
        
        return errors;
    }
    
    public Vector<Log> checkNodeDeclConsistency(Plan p)
    {
        Vector<Log> errors = new Vector<Log>();

        NodeList nodes = p.getNodes();
        for(Node n : nodes)
            {
                Action a = n.getAction();
                // Make sure we have a declarable type (i.e. Command, Lookup, etc)
                if (a == null)
                    continue;
                if (a.getType() != ActionType.Command &&
                    a.getType() != ActionType.Lookup)
                    continue;
            
                // Add a warning if action ID is not statically determined
                String id = a.getID().exprToStaticString();
                if (id == null)
                    {
                        errors.add(nonStaticNameMessage(a.getID(), n));
                        continue;
                    }
            
                GlobalDecl d = null;
                if (p.getDecls() != null)
                    d = p.getDecls().findCallById(id);
            
                if (d == null)
                    {
                        errors.add(noDeclarationMatchMessage(n));
                    }
                else
                    {
                        Log result = compareCalls(n.getAction(),d,n);
                        if (result != null)
                            errors.add(result);
                    }
            }
        return errors;
    }
    
    public Vector<Log> checkVarAccessibility(Plan p)
    {
        Vector<Log> errors = new Vector<Log>();
        for (Node n : p.getNodes())
            {
                for (Condition c : n.getConditions())
                    {
                        Expr condExpr = c.getExpr();
                        errors.addAll(checkVarsInExpr(condExpr, n, "in " + c.getType().toString() + " Condition of node " + n.getID()));
                        errors.addAll(checkLookupsInExpr(condExpr, n, p, "in " + c.getType().toString() + " Condition of node " + n.getID()));
                        errors.addAll(checkExprType(condExpr, n, p, ExprType.Bool).messages);
                    }
                Action a = n.getAction();
                if (a.getType().equals(ActionType.Assignment))
                    {
                        Expr idExpr = a.getRet();
                        Expr assignExpr = a.getArgs().elementAt(0);

                        Vector<Log> assignErrors = new Vector<Log>();
                        assignErrors.addAll(checkVarsInExpr(idExpr, n, "in LHS Assignment of node " + n.getID()));
                        assignErrors.addAll(checkVarsInExpr(assignExpr, n, "in RHS Assignment of node " + n.getID()));
                        assignErrors.addAll(checkLookupsInExpr(assignExpr, n, p, "in RHS Assignment of node " + n.getID()));
                
                        // Don't bother checking types if var issues have been found
                        if (!assignErrors.isEmpty())
                            {
                                errors.addAll(assignErrors);
                                continue;
                            }
                
                        ExprTypeWithLog lhsType = checkExprType(idExpr, n, p, ExprType.A);
                        ExprTypeWithLog rhsType = checkExprType(assignExpr, n, p, ExprType.A);
                        if (lhsType.exprType == null || rhsType.exprType == null || !lhsType.exprType.equals(rhsType.exprType))
                            {
                                // Get Var instance from LHS
                                LeafExpr e = (LeafExpr)findSubExprs(idExpr, ExprElement.Var).elementAt(0);
                                Var v = n.findVarInScope(e.getID());
                                errors.add(assignmentMismatchMessage(v, rhsType.exprType, n));
                            }
                    }
            }
        return errors;
    }
    
    private ExprList findSubExprs(Expr e, ExprElement elem)
    {
        ExprList es = new ExprList();
        
        if (e.getElem().equals(elem))
            es.add(e);
        if (e.getLeftChild() != null)
            es.addAll(findSubExprs(e.getLeftChild(), elem));
        if (e.getRightChild() != null)
            es.addAll(findSubExprs(e.getRightChild(), elem));
        
        return es;
    }
    
    private Vector<Log> checkVarsInExpr(Expr ex, Node nodeContext, String message)
    {
        Vector<Log> errors = new Vector<Log>();
        
        // Find all Var sub expressions in ex, and check they are both accessible the right type
        for (Expr e : findSubExprs(ex, ExprElement.Var))
            {
                // Now we have a list of Var references from node n
                LeafExpr le = (LeafExpr)e;
                ExprType leType = le.getType();
                if (leType.equals(ExprType.NodeState) ||
                    leType.equals(ExprType.NodeTimepointValue) ||
                    leType.equals(ExprType.NodeOutcome) ||
                    leType.equals(ExprType.NodeFailureType) ||
                    leType.equals(ExprType.NodeCommandHandle)) {
                        // Ignore Const type, check Var type that node actually exists
                        if (le.getElem().equals(ExprElement.Var))
                            {
                                String name = le.getID();
                                // Current and children nodes
                                if (nodeContext.isNamed(name) || nodeContext.hasChildNamed(name))
                                    continue;
                                // Parent and sibling nodes
                                Node parent = nodeContext.getParent();
                                if (parent != null && (parent.isNamed(name) || parent.hasChildNamed(name)))
                                    continue;
                    
                                // Didn't find a nearby node with correct name
                                errors.add(inaccessibleNodeMessage(nodeContext, name));
                            }
                        continue;
                    }

                Var v = nodeContext.findVarInScope(le.getID());
                if (v == null)
                    {
                        // Couldn't find Var definition
                        errors.add(variableMismatchMessage(le, message));
                        continue;
                    }
            
                // Check that variable is used properly
                boolean isIDMatch = v.getID().equals(le.getID());
                boolean isTypeMatch = v.getType().equals(leType);
                if (isIDMatch && !isTypeMatch)
                    errors.add(variableMismatchMessage(v,le, message));
            
                boolean isUsedAsArray = false;
                if (le.getParent() != null)
                    isUsedAsArray = le.getParent().getElem().equals(ExprElement.ArrayElem);
                // XOR checks is VarArray is not surrounded with ArrayElem,
                // or non array Var is accessed like an array
                if (v instanceof VarArray ^ isUsedAsArray)
                    errors.add(arrayMisuseMessage(v, ex));
            }
        
        return errors;
    }
    
    private Vector<Log> checkLookupsInExpr(Expr ex, Node nodeContext, Plan planContext, String message)
    {
        Vector<Log> errors = new Vector<Log>();
        
        // Find all Lookup sub expressions in ex, and check they are both accessible the right type
        for (Expr e : findSubExprs(ex, ExprElement.Lookup))
            {           
                LookupExpr le = (LookupExpr)e;
                if (le.getType() != null)
                    if (le.getType().equals(ExprType.NodeState) ||
                        le.getType().equals(ExprType.NodeTimepointValue) ||
                        le.getType().equals(ExprType.NodeOutcome) ||
                        le.getType().equals(ExprType.NodeFailureType) ||
                        le.getType().equals(ExprType.NodeCommandHandle))
                        continue; // NodeState/Outcome et al should not exist for lookups
            
                if (le.getAction() == null)
                    continue;

                String id = le.getStaticID();
                if (id == null)
                    {
                        errors.add(nonStaticNameMessage(ex, nodeContext));
                        continue;
                    }
            
                GlobalDecl d = planContext.findDecl(id);
                if (d == null)
                    {
                        // Couldn't find Lookup definition
                        errors.add(noLookupDeclarationMessage(le, message));
                        continue;
                    }

                // Check that lookup is used properly (well typed)
                Log result = compareCalls(le.getAction(), d, nodeContext);
                if (result != null)
                    errors.add(result);
            }
        
        return errors;
    }
    
    private Log compareCalls(Action a, GlobalDecl d, Node n)
    {
        // Compare action type
        switch (a.getType()) {
        case Command:
            if (d.getType() != CallType.Command)
                return declarationMismatchMessage(a, n, d);
            break;

        case LibraryCall:
            if (d.getType() != CallType.LibraryCall)
                return declarationMismatchMessage(a, n, d);
            break;
        
        case Lookup:
            if (d.getType() != CallType.Lookup)
                return declarationMismatchMessage(a, n, d);
            break;

        default:
            break;
        }
        
        // Compare arguments
        ExprList aArgs = a.getArgs();
        VarList dArgs = d.getArgs();
        
        if (aArgs.size() != dArgs.size())
            return declarationMismatchMessage(a, n, d);
        
        for (int i = 0; i < aArgs.size(); i++) {
            if (!aArgs.elementAt(i).getType().equals(dArgs.elementAt(i).getType()))
                return declarationMismatchMessage(a, n, d);
        }
        
        // Compare return values
        ExprList aRets = a.getArgs();
        VarList dRets = d.getArgs();
        
        if (aRets.size() > dRets.size())
            return declarationMismatchMessage(a, n, d);
        
        for (int i = 0; i < aRets.size(); i++) {
            if (!aRets.elementAt(i).getType().equals(dRets.elementAt(i).getType()))
                return declarationMismatchMessage(a, n, d);
        }

        return null;
    }
    
    private ExprTypeWithLog checkExprType(Expr e, Node nodeContext, Plan planContext, ExprType possibleType)
    {
        ExprTypeWithLog intersect = new ExprTypeWithLog();
        
        if (e == null)
            return intersect;

        ExprType eType = e.getTypeByElem(planContext);
        
        // Type check current expression
        intersect.exprType = Expr.typeIntersection(possibleType, eType);
        if (intersect.exprType == null)
            {
                intersect.messages.add(expressionTypeErrorMessage(e, eType, possibleType));
                return intersect;
            }
        
        // Recursively type check subpieces of expression
        Expr l = e.getLeftChild();
        Expr r = e.getRightChild();
        
        ExprTypeWithLog lType = checkExprType(l, nodeContext, planContext, e.getChildTypeByElem());
        ExprTypeWithLog rType;

        // For ArrayElem expression, LHS should be a Var and RHS is the index.
        // It seems to be the only binary expression with differing child expr types.
        // So, it gets a special block of code
        if (e.getElem().equals(ExprElement.ArrayElem))
            {
                rType = checkExprType(r, nodeContext, planContext, ExprType.Num);
            
                Var v = planContext.findVar(((LeafExpr)l).getID());
                if (v != null && !(v instanceof VarArray))
                    lType.messages.add(arrayMisuseMessage(v,e));
            
                lType.messages.addAll(rType.messages);
                return lType;
            }
        
        rType = checkExprType(r, nodeContext, planContext, e.getChildTypeByElem());
        
        // For cases like Eq, we need to check that children expressions have the same type
        if (l != null && r != null && Expr.typeIntersection(lType.exprType, rType.exprType) == null)
            intersect.messages.add(expressionTypeErrorMessage(r, lType.exprType, rType.exprType));

        return intersect;
    }
    
    private class ExprTypeWithLog
    {
        public ExprType exprType;
        public Vector<Log> messages = new Vector<Log>();
    }
    
    // Error message list: These could possibly be combined
    
    public Log nameConflictMessage(GlobalDecl a, GlobalDecl b)
    {
        return Log.error("Name conflict between " + a.toString() + " and " + b.toString());
    }
    
    public Log nonStaticNameMessage(Expr e, Node n)
    {
        return Log.warning("Cannot statically determine correctness of name " + e.toString() + " in " + n.getID());
    }
    
    public Log noDeclarationMatchMessage(Node n)
    {
        return Log.warning("No declaration matches " + n.getAction().toString() + " in " + n.toString());
    }
    
    public Log declarationMismatchMessage(Action a, Node n, GlobalDecl d)
    {
        return Log.error(a.toString() + " in " + n.getID()
                         + " does not match "+ d.toString());
    }
    
    public Log inaccessibleNodeMessage(Node n, String name)
    {
        return Log.warning("Node " + name + " is not accessible from scope of node " + n.getID());
    }
    
    public Log expressionTypeErrorMessage(Expr e, ExprType type, ExprType expectedType)
    {
        String hasTypeMessage =
            (type == null) ? "unknown type" : "type " + type.toString();
        String expectedMessage =
            (expectedType == null) ? "unknown expected type" : "expected type" + expectedType.toString();
        return Log.error("Expr " + e.toString() + " with " + hasTypeMessage
                         + " does not match " + expectedMessage);
    }
    
    public Log variableMismatchMessage(LeafExpr le, String extra)
    {
        String s = "Could not find variable definition to match expression " + le.getID();
        if (extra != null && !extra.equals(""))
            s += " " + extra;
        return Log.error(s);
    }
    
    public Log variableMismatchMessage(Var v, LeafExpr le, String extra)
    {
        if (v == null)
            return variableMismatchMessage(le, extra);
        String s = "Var " + v.getID() + " with type " + v.getType().toString()
            + " does not match expected type " + le.getType().toString();
        if (extra != null && !extra.equals(""))
            s += " " + extra;
        return Log.error(s);
    }

    public Log noLookupDeclarationMessage(LookupExpr le, String extra)
    {
        String s = "No declaration matches " + le.getAction().toString();
        if (extra != null && !extra.equals(""))
            s += " " + extra;
        return Log.warning(s);
    }
    
    public Log lookupMismatchMessage(GlobalDecl d, LookupExpr le, String extra)
    {
        if (d == null)
            return noLookupDeclarationMessage(le, extra);
        String s = "Found Lookup " + d.getID() + ", but type " + d.getType().toString()
            + " does not match expected type " + le.getType().toString();
        if (extra != null && !extra.equals(""))
            s += " " + extra;
        return Log.error(s);
    }

    public Log assignmentMismatchMessage(Var v, ExprType e, Node n)
    {
        String s = "Var " + v.toString()
            + " does not match RHS expected type " + e.toString()
            + " in " + n.toString();
        return Log.error(s);
    }

    public Log arrayMisuseMessage(Var v, Expr e)
    {
        if (!(v instanceof VarArray))
            return Log.error(v.getID() + " in expression " + e.toString() + " is not an array.");
        else
            return Log.error(v.getID() + " in expression " + e.toString() + " needs to be accessed as a single array element.");
    }
}
