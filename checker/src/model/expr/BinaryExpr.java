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

public class BinaryExpr extends Expr {

        public Expr getLeftChild() { return leftChild; }
        public Expr getRightChild() { return rightChild; }
        public void setLeftChild(Expr l) { this.leftChild = l; if (l != null) l.setParent(this); }
        public void setRightChild(Expr r) { this.rightChild = r; if (r != null) r.setParent(this); }

	public BinaryExpr(ExprElement e)
	{
		setElem(e);
		leftChild = null;
		rightChild = null;
	}
	
	public BinaryExpr(ExprElement e, ExprType ct)
	{
		setElem(e);
		leftChild = null;
		rightChild = null;
		setChildType(ct);
	}
	
	public BinaryExpr(ExprElement e, Expr l, Expr r)
	{
		setElem(e);
		setLeftChild(l);
		setRightChild(r);
	}

	public BinaryExpr(ExprElement e, Expr l, Expr r, ExprType ct)
	{
		setElem(e);
		setLeftChild(l);
		setRightChild(r);
		setChildType(ct);
	}

	// Get possible result types for current Expr
	public ExprType getTypeByElem(Plan p)
	{
		switch (this.getElem())
		{
		case Plus:
		case Minus:
		case Mult:
		case Div:
			return ExprType.Num;
		case Eq:
		case NEq:
		case LT:
		case GT:
		case LEq:
		case GEq:
		case Or:
		case And:
		case XOr:
			return ExprType.Bool;
		case Concat:
			return ExprType.Str;
		case ArrayElem:
		case Alias:
			return ExprType.A;
		}
		
		return null;
	}

	public ExprType getChildTypeByElem()
	{
		switch (this.getElem())
		{
		case Plus:
		case Minus:
		case Mult:
		case Div:
		case LT:
		case GT:
		case LEq:
		case GEq:
			return ExprType.Num;
		case Or:
		case And:
		case XOr:
			return ExprType.Bool;
		case Concat:
			return ExprType.Str;
		case Eq:
		case NEq:
		case ArrayElem:
		case Alias:
			return ExprType.A;
		}
		
		return null;
	}

	@Override
	public String toString()
	{
		String elemStr = "null", rightStr ="null", leftStr = "null";
		if(leftChild != null)
			leftStr = leftChild.toString();
		if(rightChild != null)
			rightStr = rightChild.toString();
		if(getElem() != null)
			elemStr = getElem().toString();
		
		return "(" + elemStr + " " + leftStr + " " + rightStr + ")";
	}
}
