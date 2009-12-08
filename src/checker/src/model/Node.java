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

import model.Action.*;

public class Node {
	
	private String id;
	private Node parent;
	private NodeList children = new NodeList();
	private VarList varDefs = new VarList();
	private ConditionList conditions = new ConditionList();
	private Action action = new Action(ActionType.Empty);
	
	public String getID() {
		return id;
	}
	public void setID(String id) {
		this.id = id;
	}
	public Node getParent() {
		return parent;
	}
	public void setParent(Node parent) {
		this.parent = parent;
	}
	public NodeList getChildren() {
		return children;
	}
	public void setChildren(NodeList children) {
		this.children = children;
	}
	public void addChild(Node c) {
		this.children.add(c);
	}
	public VarList getVarDefs() {
		return varDefs;
	}
	public void setVarDefs(VarList varDefs) {
		this.varDefs = varDefs;
	}
	public void addVarDef(Var v) {
		this.varDefs.add(v);
	}
	public ConditionList getConditions() {
		return conditions;
	}
	public void setConditions(ConditionList conditions) {
		this.conditions = conditions;
	}
	public void addCondition(Condition c) {
		this.conditions.add(c);
	}
	public Action getAction() {
		return action;
	}
	public void setAction(Action action) {
		this.action = action;
	}
	
	public boolean isNamed(String name) {
		if (name == null || id == null)
			return false;
		return id.equals(name);
	}
	
	public boolean hasChildNamed(String name)
	{
		if (name == null || children == null)
			return false;
		for (Node child : children)
		{
			if (child.isNamed(name))
				return true;
		}
		return false;
	}
	
	public Var findVarInScope(String id)
	{
		VarList vs = getVarDefs();
		for (int i = 0; i < vs.size(); i++)
		{
			Var v = vs.elementAt(i);
			if (v.getID().equals(id))
				return v;
		}
		
		if (getParent() == null)
			return null;
		
		return getParent().findVarInScope(id);
	}

	public void print()
	{
		System.out.println(this.toString());
		if (varDefs != null)
		{
			for (Var v : varDefs)
				System.out.println("  " + v.toString() + " Variable");
		}
		if (conditions != null)
		{
			for (Condition c : conditions)
				System.out.println("  " + c.getExpr().toString() + " :: " + c.getType().toString() + " Condition");
		}
	}
	
	@Override
	public String toString()
	{
		if (getAction() != null)
			 return (id + " :: " + action.getType().toString() + " Node");
		else
			return (id + " :: UnknownType Node");
	}
}
