/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

/*
 *   Lookups only appear in expressions, but they are structured like other actions
 *   So, they get their own class.
 */

import model.*;

public class LookupExpr extends Expr {
	
	Action action;
	
	public Action getAction() { return action; }
	public String getStaticID() { if (action == null) return null; return action.getID().exprToStaticString(); }
	public Expr getLeftChild() { return null; }
	public Expr getRightChild()	{ return null; }
	public void setLeftChild(Expr l)
	{
		if (l == null)
			return;
		System.out.println("WARNING: Trying to add expression " + l.getElem().toString()
			+ " as left child to " + this.getElem().toString()
			+ "\n | This is not allowed for LookupExpr.");
	}
	public void setRightChild(Expr r)
	{
		if (r == null)
			return;
		System.out.println("WARNING: Trying to add expression " + r.getElem().toString()
			+ " as right child to " + this.getElem().toString()
			+ "\n | This is not allowed for LookupExpr.");
	}

	public LookupExpr(Expr i)
	{
		action = new Action(Action.ActionType.Empty, i);
		setElem(ExprElement.Lookup);
		leftChild = null;
		rightChild = null;
	}
	
	public LookupExpr(Expr i, ExprType t)
	{
		action = new Action(Action.ActionType.Empty, i);
		setElem(ExprElement.Lookup);
		setType(t);
		leftChild = null;
		rightChild = null;
	}
	
	public LookupExpr(Action a)
	{
		action = a;
		setElem(ExprElement.Lookup);
		leftChild = null;
		rightChild = null;
	}
	
	public LookupExpr(Action a, ExprType t)
	{
		action = a;
		setElem(ExprElement.Lookup);
		setType(t);
		leftChild = null;
		rightChild = null;
	}
	
	public ExprType getTypeByElem(Plan p)
	{
		// We will catch non-static ID issues elsewhere, so pass back an all-accepting value
		if (getStaticID() == null)
			return ExprType.A;
		for (GlobalDecl d : p.getDecls())
		{
			if (d.getID().equals(getStaticID()))
				return d.getRets().elementAt(0).getTypeAsExprType(); //return first value type of return list
		}
		return ExprType.A;
	}
	
	public ExprType getChildTypeByElem()
	{
		return null;
	}
	
	@Override
	public String toString()
	{
		return getAction().getID().toString();
	}
}
