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

import model.expr.*;

/*
 * Notes:
 *   List Nodes have no 'action', so they can be marked as Empty
 *   Assignment nodes (Var x = Expr e;) set e as the argument, x as the return
 */

public class Action {
	
	public enum ActionType {Command, LibraryCall, Assignment, Lookup, Update, Empty};
	
	private ActionType type;
	private Expr id;
	private ExprList arguments;
	private Expr returnEx;
	
	public ActionType getType() { return type; }
	public Expr       getID()   { return id; }
	public ExprList   getArgs() { return arguments; }
	public Expr       getRet() { return returnEx; }
	
	public Action(ActionType t)
	{
		type = t;
		id = new LeafExpr("", Expr.ExprElement.Const, Expr.ExprType.Str);
		arguments = new ExprList();
		returnEx = null;
	}

	public Action(ActionType t, Expr i)
	{
		type = t;
		id = i;
		arguments = new ExprList();
		returnEx = null;
	}

	public Action(ActionType t, Expr i, ExprList args)
	{
		type = t;
		id = i;
		arguments = args;
		returnEx = null;
	}

	public Action(ActionType t, Expr i, ExprList args, Expr ret)
	{
		type = t;
		id = i;
		arguments = args;
		returnEx = ret;
	}

	public void print()
	{
		System.out.println(id + " :: " + type);
		if (arguments != null)
		{
			System.out.println("  Arguments = ");
			for (Expr arg : arguments)
				System.out.println("    " + arg.toString());
		}
		if (returnEx != null)
		{
			System.out.println("  Return = ");
			System.out.println("    " + returnEx.toString());
		}
	}
	
	@Override
	public String toString()
	{
		String actionID;
		if (id.exprToStaticString() == null)
			actionID = id.toString();
		else
			actionID = id.exprToStaticString();
		return type.toString() + " " + actionID + "(" + arguments.toTypeString() + ")";
	}
}
