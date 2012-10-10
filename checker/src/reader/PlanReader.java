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
import model.*;
import model.Var.VarType;
import model.Action.ActionType;
import model.expr.*;
import java.util.Vector;

public class PlanReader {
	public static final String NodeRootText = "Node";
	public static final String NodeIdText = "NodeId";
	public static final String ActionNameText = "Name";
	public static final String NodeBodyText = "NodeBody";
	public static final String ArgumentCallText = "Arguments";
	public static final String ReturnCallText = "Returns";
	public static final String AliasCallText = "Alias";
	public static final String VarListDeclarationText = "VariableDeclarations";
	public static final String VarDeclarationText = "DeclareVariable";
	public static final String ArrayDeclarationText = "DeclareArray";
	public static final String CommandCallText = "Command";
	public static final String FunctionCallText = "Function";
	public static final String LibraryNodeCallText = "LibraryNodeCall";
	public static final String LookupCallText = "Lookup";
	public static final String AssignmentText = "Assignment";
	public static final String UpdateText = "Update";
	public static final String NodeListText = "NodeList";
	public static final String BooleanTrueText = "true";
	public static final String BooleanFalseText = "false";
	public static final String ConditionText = "Condition";
	public static final String StartCondText = "StartCondition";
	public static final String EndCondText = "EndCondition";
	public static final String PreCondText = "PreCondition";
	public static final String PostCondText = "PostCondition";
	public static final String RepeatCondText = "RepeatCondition";
	public static final String InvarCondText = "InvariantCondition";
	public static final String SkipCondText = "SkipCondition";

	private ExprReader exprReader = new ExprReader();
	
	public Plan buildPlan(IXMLElement xml)
	{
		return buildPlan(xml, null);
	}
	
	private Plan buildPlan(IXMLElement xml, Node parent)
	{
		Plan plan = new Plan();
		
		if (xml == null)
			return plan;

		// If we are currently looking at a node, extract it
		if (xml.getName().equals(NodeRootText))
		{
			Node currentNode = xmlToNode(xml);
			
			// Attach to parent node
			if (parent != null)
			{
				currentNode.setParent(parent);
				parent.addChild(currentNode);
			}

			plan.addNode(currentNode);
			
			// Set up children of this node
			for (IXMLElement child : getChildren(xml))
				plan.combineWith(buildPlan(child, currentNode));
		}
		else
		{
			// We were not looking at a node, so continue drilling down
			for (IXMLElement child : getChildren(xml))
				plan.combineWith(buildPlan(child, parent));
		}
		
		return plan;
		
	}
	
	public Node xmlToNode(IXMLElement xml)
	{
		// Need a valid node IXMLElement
		if (xml == null || !xml.getName().equals(NodeRootText))
			return null;

		Node n = new Node();

		for (IXMLElement component : getChildren(xml))
		{
			String compText = component.getName();
			if (compText.equals(NodeIdText))
				n.setID(component.getContent());
			else if (compText.equals(VarListDeclarationText))
				n.setVarDefs(buildVarDefList(component));
			else if (compText.contains(ConditionText))
				n.addCondition(buildCondition(component));
			else if (compText.equals(NodeBodyText))
				n.setAction(buildAction(component.getChildAtIndex(0)));
			// TODO: No other options, right?
		}
		
		return n;
	}
	
	public Condition buildCondition(IXMLElement xml)
	{
		if (xml == null || !xml.getName().contains(ConditionText))
			return null;
		
		Condition.ConditionType type = Condition.ConditionType.Start;
		String text = xml.getName();
		if (text.equals(StartCondText))
			type = Condition.ConditionType.Start;
		if (text.equals(EndCondText))
			type = Condition.ConditionType.End;
		if (text.equals(PreCondText))
			type = Condition.ConditionType.Pre;
		if (text.equals(PostCondText))
			type = Condition.ConditionType.Post;
		if (text.equals(InvarCondText))
			type = Condition.ConditionType.Invar;
		if (text.equals(RepeatCondText))
			type = Condition.ConditionType.Repeat;
		if (text.equals(SkipCondText))
			type = Condition.ConditionType.Skip;
		
		return new Condition(type, exprReader.xmlToExpr(xml.getChildAtIndex(0)));	
	}

	public VarList buildVarDefList(IXMLElement xml)
	{
		VarList list = new VarList();
		
		if (xml == null || !xml.getName().equals(VarListDeclarationText))
			return list;

		for (IXMLElement var : getChildren(xml))
		{
			Var v = buildVarDef(var);
			if (v != null)
				list.add(v);
		}
		
		return list;
	}
	
	public Var buildVarDef(IXMLElement xml)
	{
		if (xml == null)
			return null;
		if (!xml.getName().equals(VarDeclarationText) && !xml.getName().equals(ArrayDeclarationText))
			return null;
		
		String name = xml.getChildAtIndex(0).getContent();
		String type = xml.getChildAtIndex(1).getContent();
		VarType varType;
		
		if (type.equals("Boolean"))
			varType = VarType.B;
		else if (type.equals("Integer"))
			varType = VarType.I;
		else if (type.equals("Real"))
			varType = VarType.R;
		else //if (type.equals("String"))
			varType = VarType.S;
		
		if (xml.getName().equals(VarDeclarationText))
			return new Var(name, varType);
		
		String arraySize = xml.getChildAtIndex(2).getContent();
			return new VarArray(name, varType, Integer.parseInt(arraySize));
	}
	
	public Action buildAction(IXMLElement xml)
	{
		if (xml == null)
			return new Action(ActionType.Empty);
		
		if (xml.getName().equals(AssignmentText))
			return buildAssignment(xml);
		if (xml.getName().equals(NodeListText))
			return new Action(ActionType.Empty);

		ActionType nodeType = ActionType.Command;
		Expr id = new LeafExpr("", Expr.ExprElement.Const, Expr.ExprType.Str);
		ExprList args = new ExprList();
		Expr ret = null;
		
		if (xml.getName().equals(CommandCallText))
			nodeType = ActionType.Command;
		if (xml.getName().equals(FunctionCallText))
			nodeType = ActionType.Function;
		if (xml.getName().equals(LibraryNodeCallText))
			nodeType = ActionType.LibraryCall;
		if (xml.getName().startsWith(LookupCallText))
			nodeType = ActionType.Lookup;
		// TODO: Add Update stuff
		
		for (IXMLElement childNode : getChildren(xml))
		{
			String childText = childNode.getName();
			if (childText.equals(ArgumentCallText))
			{
				for (IXMLElement child : getChildren(childNode))
				{
					args.add(exprReader.xmlToExpr(child));
				}
			}
			else if (childText.equals(ActionNameText) || childText.equals(NodeIdText))
			{
				IXMLElement nameNode = childNode;
				if (childNode.hasChildren())
					nameNode = childNode.getChildAtIndex(0);
				
				if (nameNode.getContent() == null)
					id = exprReader.xmlToExpr(nameNode);
				else
					id = new LeafExpr(nameNode.getContent(), Expr.ExprElement.Const, Expr.ExprType.Str);
			}
			else if (childText.equals(AliasCallText))
			{
				args.add(exprReader.xmlToExpr(childNode));
			}
			else
			{
				ret = exprReader.xmlToExpr(childNode);
			}
		}

		return new Action(nodeType, id, args, ret);
	}
	
	// Assignments work a bit differently than other actions.
	public Action buildAssignment(IXMLElement xml)
	{
		if (xml == null)
			return null;
		
		// Don't create anything unless this is an assignment
		if (!xml.getName().contains(AssignmentText))
			return null;
		
		// Assignments don't have a specific 'name' like commands
		Expr id = new LeafExpr("", Expr.ExprElement.Const);
		
		Expr lhs = exprReader.xmlToExpr(xml.getChildAtIndex(0));
		ExprList rhs = new ExprList(exprReader.xmlToExpr(xml.getChildAtIndex(1)));
		
		return new Action(ActionType.Assignment, id, rhs, lhs);
	}
	
	// IXMLElement.getChildren() only returns Vector. Having a typed vector is nicer, so we type cast here.
	@SuppressWarnings("unchecked")
	private Vector<IXMLElement> getChildren(IXMLElement elem)
	{
		Vector<IXMLElement> children = elem.getChildren();
		return children;
	}
}
