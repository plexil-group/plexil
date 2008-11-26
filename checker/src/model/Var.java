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

package model;

import model.expr.*;

public class Var
{
	private String id;
	private VarType type; 
	private VarMod mod;
	
	public String getID() { return id; }
	public VarType getType() { return type; }
	public Expr.ExprType getTypeAsExprType() { return type.toExprType(); }
	public VarMod getMod() { return mod; }

	public Var (VarType t) { id = ""; type = t; mod = VarMod.None;}
	public Var (String i, VarType t) { id = i; type = t; mod = VarMod.None;}
	public Var (VarType t, VarMod m) { id = ""; type = t; mod = m;}
	public Var (String i, VarType t, VarMod m) { id = i; type = t; mod = m;}

	public enum VarType {I, R, S, B;
		public boolean equals(Expr.ExprType et)
		{
			if (et.equals(Expr.ExprType.A))
				return true;
			switch(this)
			{
			case I:
			case R:
				if (et.equals(Expr.ExprType.Num))
					return true;
			case B:
				if (et.equals(Expr.ExprType.Bool))
					return true;
			case S:
				if (et.equals(Expr.ExprType.Str))
					return true;
			}
			return false;
		}
		public Expr.ExprType toExprType()
		{
			switch(this)
			{
			case I:
			case R:
				return Expr.ExprType.Num;
			case B:
				return Expr.ExprType.Bool;
			case S:
				return Expr.ExprType.Str;
			default:
				return null;
			}
		}
		@Override
		public String toString()
		{
			switch(this)
			{
			case I:
				return "Int";
			case R:
				return "Real";
			case B:
				return "Bool";
			case S:
				return "Str";
			}
			return "";
		}
	};
	public enum VarMod {None, In, InOut};
	
	@Override
	public String toString()
	{
		String result = id;
		if (!id.equals(""))
			result += " :: ";
		
		if (mod != VarMod.None)
			result += mod.toString() + " ";
		
		return result + type.toString();
	}

	public String toTypeString()
	{
		return type.toString();
	}
}
