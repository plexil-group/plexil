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

public class LeafExpr extends Expr {

	private String id = "";
	
	public String getID() { return id; }
	public Expr getLeftChild() { return null; }
	public Expr getRightChild()	{ return null; }
	public void setLeftChild(Expr l)
	{
		if (l == null)
			return;
		System.out.println("WARNING: Trying to add expression " + l.getElem().toString()
			+ " as left child to " + this.getElem().toString()
			+ "\n | This is not allowed for LeafExpr.");
	}
	public void setRightChild(Expr r)
	{
		if (r == null)
			return;
		System.out.println("WARNING: Trying to add expression " + r.getElem().toString()
			+ " as right child to " + this.getElem().toString()
			+ "\n | This is not allowed for LeafExpr.");
	}

	public LeafExpr(String i, ExprElement e)
	{
		id = i;
		setElem(e);
		leftChild = null;
		rightChild = null;
	}
	
	public LeafExpr(String i, ExprElement e, ExprType t)
	{
		id = i;
		setElem(e);
		setType(t);
		leftChild = null;
		rightChild = null;
	}
	
	public ExprType getTypeByElem(Plan p)
	{
        ExprType t = getType();

		switch(getElem())
		{
		case Var:
			if (t.equals(ExprType.NodeState) ||
                t.equals(ExprType.NodeTimepointValue) ||
                t.equals(ExprType.NodeOutcome) ||
                t.equals(ExprType.NodeFailureType) ||
                t.equals(ExprType.NodeCommandHandle))
				return t;
			for (Var v : p.getVars()) {
				if (v.getID().equals(id))
					return v.getTypeAsExprType();
			}
			break;

		case Const:
			return t;
		}
		return ExprType.A;
	}

	public ExprType getChildTypeByElem() { return null; }

	@Override
	public String toString()
	{
		return "(" + getElem().toString() + " " + id + ")";
	}
}
