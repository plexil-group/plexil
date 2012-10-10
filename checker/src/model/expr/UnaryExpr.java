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

import model.*;

public class UnaryExpr extends Expr {
	public UnaryExpr(ExprElement e)
	{
		setElem(e);
		leftChild = null;
		rightChild = null;
	}
	
	public UnaryExpr(ExprElement e, Expr l)
	{
		setElem(e);
		setLeftChild(l);
		rightChild = null;
	}
	
	public UnaryExpr(ExprElement e, Expr l, ExprType t)
	{
		setElem(e);
		setLeftChild(l);
		rightChild = null;
		setChildType(t);
	}
	
	public Expr getLeftChild() { return leftChild; }
	public Expr getRightChild() { return null; }
	public void setLeftChild(Expr l)
	{
		leftChild = l;
		l.setParent(this);
	}
	public void setRightChild(Expr r)
	{
		if (r == null)
			return;
		System.out.println("WARNING: Trying to add expression " + r.getElem().toString()
			+ " as right child to " + this.getElem().toString()
			+ "\n | UnaryExpr can only access left child.");
	}


	// Get possible result types for current Expr
	public ExprType getTypeByElem(Plan p)
	{
		switch (this.getElem())
		{
		case Abs:
		case Sqrt:
			return ExprType.Num;
		case Not:
		case IsKnown:
			return ExprType.Bool;
		}
		
		return null;
	}

	// Possible types for child element(s)
	public ExprType getChildTypeByElem()
	{
		switch (this.getElem())
		{
		case Sqrt:
		case Abs:
			return ExprType.Num;
		case Not:
			return ExprType.Bool;
		case IsKnown:
			return ExprType.A;
		}
		
		return null;
	}

	@Override
	public String toString()
	{
		return "(" + getElem().toString() + " " + leftChild.toString() + ")";
	}
}
