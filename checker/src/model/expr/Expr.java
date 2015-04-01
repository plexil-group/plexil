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

package model.expr;

import model.*;

public abstract class Expr { 
	// Leaf
	public static Expr Lookup(String id) { return new LeafExpr(id, ExprElement.Lookup); }
	public static Expr Var(String id) { return new LeafExpr(id, ExprElement.Var); }
	public static Expr Const(ExprType t) { return new LeafExpr("", ExprElement.Const, t); }
	// Unary
	public static Expr Sqrt(Expr l)  { return new UnaryExpr(ExprElement.Sqrt, l); }
	public static Expr Abs(Expr l)  { return new UnaryExpr(ExprElement.Abs, l); }
	public static Expr Ceil(Expr l)  { return new UnaryExpr(ExprElement.Ceil, l); }
	public static Expr Floor(Expr l)  { return new UnaryExpr(ExprElement.Floor, l); }
	public static Expr Round(Expr l)  { return new UnaryExpr(ExprElement.Round, l); }
	public static Expr Trunc(Expr l)  { return new UnaryExpr(ExprElement.Trunc, l); }
	public static Expr RealToInt(Expr l)  { return new UnaryExpr(ExprElement.RealToInt, l); }
	public static Expr Strlen(Expr l)  { return new UnaryExpr(ExprElement.Strlen, l); }
	public static Expr Not(Expr l)  { return new UnaryExpr(ExprElement.Not, l); }
	public static Expr IsKnown(Expr l)  { return new UnaryExpr(ExprElement.IsKnown, l); }
	// Binary
	public static Expr Eq(Expr l, Expr r) { return new BinaryExpr(ExprElement.Eq, l, r); }
	public static Expr Eq(ExprType t, Expr l, Expr r) { return new BinaryExpr(ExprElement.Eq, l, r, t); }
	public static Expr NEq(Expr l, Expr r) { return new BinaryExpr(ExprElement.NEq, l, r); }
	public static Expr NEq(ExprType t, Expr l, Expr r) { return new BinaryExpr(ExprElement.NEq, l, r, t); }
	public static Expr LT(Expr l, Expr r) { return new BinaryExpr(ExprElement.LT, l, r); }
	public static Expr GT(Expr l, Expr r) { return new BinaryExpr(ExprElement.GT, l, r); }
	public static Expr LEq(Expr l, Expr r) { return new BinaryExpr(ExprElement.LEq, l, r); }
	public static Expr GEq(Expr l, Expr r) { return new BinaryExpr(ExprElement.GEq, l, r); }
	public static Expr ArrayElem(Expr l, Expr r) { return new BinaryExpr(ExprElement.ArrayElem, l, r); }
	public static Expr ArrayElem(ExprType t, Expr l, Expr r) { return new BinaryExpr(ExprElement.ArrayElem, l, r, t); }
	public static Expr Alias(Expr l, Expr r) { return new BinaryExpr(ExprElement.Alias, l, r); }
	public static Expr Alias(ExprType t, Expr l, Expr r) { return new BinaryExpr(ExprElement.Alias, l, r, t); }

    // N-ary *** FIXME ***

    // N > 0
	public static Expr Concat(Expr l, Expr r) { return new BinaryExpr(ExprElement.Concat, l, r); }

    // N > 1
	public static Expr Plus(Expr l, Expr r) { return new BinaryExpr(ExprElement.Plus, l, r); }
	public static Expr Minus(Expr l, Expr r) { return new BinaryExpr(ExprElement.Minus, l, r); }
	public static Expr Mult(Expr l, Expr r) { return new BinaryExpr(ExprElement.Mult, l, r); }
	public static Expr Div(Expr l, Expr r) { return new BinaryExpr(ExprElement.Div, l, r); }
	public static Expr Mod(Expr l, Expr r) { return new BinaryExpr(ExprElement.Mod, l, r); }
	public static Expr Max(Expr l, Expr r) { return new BinaryExpr(ExprElement.Max, l, r); }
	public static Expr Min(Expr l, Expr r) { return new BinaryExpr(ExprElement.Min, l, r); }
	public static Expr Or(Expr l, Expr r) { return new BinaryExpr(ExprElement.Or, l, r); }
	public static Expr And(Expr l, Expr r) { return new BinaryExpr(ExprElement.And, l, r); }
	public static Expr Xor(Expr l, Expr r) { return new BinaryExpr(ExprElement.XOr, l, r); }

	
    public enum ExprType {
        Num,
        Bool,
        Str,
        NodeState,
        NodeOutcome,
        NodeFailureType,
        NodeTimepointValue,
        NodeCommandHandle,
        A;  // A is generic, should match any other
		
        public boolean equals(Var.VarType vt) {
			if (this.equals(A))
                return true;
            switch (vt) {
            case B:
                return this.equals(ExprType.Bool);

            case S:
                return this.equals(ExprType.Str);

                // *** FIXME *** No longer true 
            case I:
            case R:
                return this.equals(ExprType.Num);

            default:
                return false;
            }
		}
	}

	public enum ExprElement {
		Const, Var, Lookup, ArrayElem, Alias,
		Plus, Minus, Mult, Div, Mod, Max, Min,
        Sqrt, Abs, Ceil, Floor, Round, Trunc, RealToInt,
		Eq, NEq, LT, GT, LEq, GEq, Not, Or, And, XOr, IsKnown,
		Concat, Strlen
		};

	protected Expr leftChild;
	protected Expr rightChild;
	private Expr parent = null;
	private ExprElement elem; 
	private ExprType type = null;
	private ExprType childType = null;
	
	public abstract Expr getLeftChild();
	public abstract Expr getRightChild();
	public abstract void setLeftChild(Expr leftChild);
	public abstract void setRightChild(Expr rightChild);
	
	public Expr getParent() {
		return parent;
	}
	public void setParent(Expr parent) {
		this.parent = parent;
	}
	public ExprType getType() {
		return type;
	}
	public void setType(ExprType type) {
		this.type = type;
	}
	public void setType(ExprElement elem) {
		switch (elem)
		{
		case Plus:
		case Minus:
		case Mult:
		case Div:
		case Abs:
		case Sqrt:
			this.type = ExprType.Num;
		case Eq:
		case NEq:
		case LT:
		case GT:
		case LEq:
		case GEq:
		case Or:
		case And:
		case XOr:
		case Not:
		case IsKnown:
			this.type = ExprType.Bool;
		case Concat:
			this.type = ExprType.Str;
		default:
			// For all other cases, we cannot specify a type without extra info
			this.type = ExprType.A;
		}
	}
	public ExprType getChildType() {
		return childType;
	}
	public void setChildType(ExprType type) {
		this.childType = type;
	}
	public void setChildType(ExprElement elem) {
		switch (elem)
		{
		case Plus:
		case Minus:
		case Mult:
		case Div:
		case Abs:
		case Sqrt:
		case LT:
		case GT:
		case LEq:
		case GEq:
			this.type = ExprType.Num;
		//case Eq:
		//case NEq:
		case Or:
		case And:
		case XOr:
		case Not:
		case IsKnown:
			this.type = ExprType.Bool;
		case Concat:
			this.type = ExprType.Str;
		default:
			// For all other cases, we cannot specify a type without extra info
			this.type = ExprType.A;
		}
	}
	public ExprElement getElem() {
		return elem;
	}
	public void setElem(ExprElement elem) {
		this.elem = elem;
		setType(elem);
		setChildType(elem);
	}
	
	// Get possible result types for current Expr
	public abstract ExprType getTypeByElem(Plan p);
	
	// Possible types for child element(s)
	public abstract ExprType getChildTypeByElem();

	public static ExprType typeIntersection(ExprType l, ExprType r)
	{
		// Check for generic case, which matches anything
		if (l != null && l.equals(ExprType.A))
			return r;
		if (r != null && r.equals(ExprType.A))
			return l;
		
		// Return empty if we have a null input
		if (l == null || r == null)
			return null;
		
		// They are the same type
		if (l.equals(r))
			return l;
		
		// Different types, so intersection is empty
		return null;
	}
	
	public String exprToStaticString()
	{
		// Need a String expression of type Const or Concat (Lookups and Vars are not static)
		if (!elem.equals(ExprElement.Const) && !elem.equals(ExprElement.Concat))
			return null;
		if (!type.equals(ExprType.Str) && !type.equals(ExprType.A))
			return null;
		
		if (elem.equals(ExprElement.Const))
		{
			LeafExpr le = (LeafExpr)this;
			return le.getID();
		}
		
		// If we have Concat expr, try concatting.
		if (getLeftChild() == null || getRightChild() == null)
			return null;
		
		String left  = getLeftChild().exprToStaticString();
		String right = getRightChild().exprToStaticString();
		
		if (left == null || right == null)
			return null;
		return left + right;
	}
	
}
