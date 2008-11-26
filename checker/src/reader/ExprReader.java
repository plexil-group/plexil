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

package reader;

import net.n3.nanoxml.*;
import model.expr.*;
import model.expr.Expr.*;
import java.util.*;

public class ExprReader {
	
	private Map<String,ExprElement> exprList = new Hashtable<String,ExprElement>();
	
	public ExprReader()
	{
		exprList.put("NodeStateValue", ExprElement.Const);
		exprList.put("NodeOutcomeValue", ExprElement.Const);
		exprList.put("NodeTimepointValue", ExprElement.Const);
		exprList.put("BooleanValue", ExprElement.Const);
		exprList.put("StringValue", ExprElement.Const);
		exprList.put("IntegerValue", ExprElement.Const);
		exprList.put("RealValue", ExprElement.Const);
		exprList.put("NodeStateVariable", ExprElement.Var);
		exprList.put("NodeOutcomeVariable", ExprElement.Var);
		exprList.put("BooleanVariable", ExprElement.Var);
		exprList.put("StringVariable", ExprElement.Var);
		exprList.put("IntegerVariable", ExprElement.Var);
		exprList.put("RealVariable", ExprElement.Var);
		exprList.put("LookupNow", ExprElement.Lookup);
		exprList.put("LookupOnChange", ExprElement.Lookup);
		exprList.put("Concat", ExprElement.Concat);
		exprList.put("ADD", ExprElement.Plus);
		exprList.put("SUB",  ExprElement.Minus);
		exprList.put("MUL",  ExprElement.Mult);
		exprList.put("DIV", ExprElement.Div);
		exprList.put("SQRT", ExprElement.Sqrt);
		exprList.put("ABS", ExprElement.Abs);
		exprList.put("EQInternal", ExprElement.Eq);
		exprList.put("EQBoolean", ExprElement.Eq);
		exprList.put("EQString", ExprElement.Eq);
		exprList.put("EQNumeric", ExprElement.Eq);
		exprList.put("GT",  ExprElement.GT);
		exprList.put("LT",  ExprElement.LT);
		exprList.put("GE", ExprElement.GEq);
		exprList.put("LE", ExprElement.LEq);
		exprList.put("NEInternal", ExprElement.NEq);
		exprList.put("NEBoolean", ExprElement.NEq);
		exprList.put("NEString", ExprElement.NEq);
		exprList.put("NENumeric", ExprElement.NEq);
		exprList.put("NOT", ExprElement.Not);
		exprList.put("OR", ExprElement.Or);
		exprList.put("AND", ExprElement.And);
		exprList.put("XOR", ExprElement.XOr);
		exprList.put("IsKnown", ExprElement.IsKnown);
		exprList.put("ArrayElement", ExprElement.ArrayElem);
		exprList.put("Alias", ExprElement.Alias);
	}
	
	public Expr xmlToExpr(IXMLElement xml)
	{
		if (xml == null)
			return null;
		
		String name = xml.getName();
		
		// Get children expressions
		Expr left = null, right = null;
		if (xml.hasChildren())
		{
			left = xmlToExpr(xml.getChildAtIndex(0));
			if (xml.getChildrenCount() > 1)
				right = xmlToExpr(xml.getChildAtIndex(1));
		}
		
		ExprType childType = null;
		if (name.contains("Boolean"))
			childType = ExprType.Bool;
		if (name.contains("String"))
			childType = ExprType.Str;
		if (name.contains("Numeric") || name.contains("Integer") || name.contains("Real"))
			childType = ExprType.Num;
		if (name.contains("Internal") || name.contains("NodeState"))
			childType = ExprType.NodeState;
		if (name.contains("NodeOutcome"))
			childType = ExprType.NodeOutcome;
		if (name.contains("NodeTimepointValue"))
			childType = ExprType.NodeTimepointValue;
		
		// I wanted to handle special cases in switch default clause,
		// but it doesn't like me passing in null Element.
		// So, prepare the special cases here.
		ExprElement elem = exprList.get(name);
		if (elem == null)
		{
			// TODO: Fix this area.
			if (name.contains("RHS"))
			{
				left.setType(childType);
				return left;
			}
			return new LeafExpr(name, ExprElement.Var);
		}
		
		switch(elem)
		{
		case Const:
		case Var:
			if (elem.equals(ExprElement.Var) && 
				(childType.equals(ExprType.NodeState) || 
                                 childType.equals(ExprType.NodeTimepointValue) || 
				 childType.equals(ExprType.NodeOutcome)) )
				return new LeafExpr(xml.getChildAtIndex(0).getContent(), elem, childType);
			return new LeafExpr(xml.getContent(), elem, childType);
		case Lookup:
			// Lookup notation in XML is <Lookup> <Name> <String>name ...
			// If lookup does not have a child, at least return something
			if (xml.getChildAtIndex(0) == null)
				return new LookupExpr(new LeafExpr(name, ExprElement.Const));
			IXMLElement nameXml = xml.getChildAtIndex(0);
			if (!nameXml.hasChildren())
				// In case one level of XML is missing
				return new LookupExpr(new LeafExpr(nameXml.getContent(), ExprElement.Const));
			// Attempt to drill down one more level
			PlanReader pr = new PlanReader();
			model.Action a = pr.buildAction(xml);
			return new LookupExpr(a);
		case Concat:
		case Plus:
		case Minus:
		case Mult:
		case Div:
		case GT:
		case LT:
		case GEq:
		case LEq:
		case Or:
		case And:
		case XOr:
			return new BinaryExpr(elem, left, right);
		case IsKnown:
		case Not:
		case Sqrt:
		case Abs:
			return new UnaryExpr(elem, left);
		case Eq:
		case NEq:
			// We need to mark the comparison type
			return new BinaryExpr(elem, left, right, childType);
		case ArrayElem:
			left = new LeafExpr(xml.getChildAtIndex(0).getContent(), ExprElement.Var);
			right = xmlToExpr(xml.getChildAtIndex(1).getChildAtIndex(0));
			return new BinaryExpr(elem, left, right);
		case Alias:
			left = new LeafExpr(xml.getChildAtIndex(0).getContent(), ExprElement.Var);
			right = xmlToExpr(xml.getChildAtIndex(1));
			Expr e = new BinaryExpr(elem, left, right);
			e.setType(right.getType());
			return e;
		default:
			// Do something for special cases
		}
		return null;
	}
}
