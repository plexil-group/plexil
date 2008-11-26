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

public class Plan {
	private NodeList nodes;
	private GlobalDeclList decls;
	private VarList vars;
	
	public Plan()
	{
		nodes = new NodeList();
		decls = new GlobalDeclList();
		vars = new VarList();
	}

	public NodeList getNodes() {
		return nodes;
	}
	public void setNodes(NodeList nodes) {
		this.nodes = nodes;
	}
	public GlobalDeclList getDecls() {
		return decls;
	}
	public void setDecls(GlobalDeclList decls) {
		this.decls = decls;
	}
	public VarList getVars() {
		return vars;
	}
	public void setVars(VarList vars) {
		this.vars = vars;
	}
	
	public void addNode(Node n)
	{
		if (n == null)
			return;
		nodes.add(n);
		if (n.getVarDefs() != null)
			vars.addAll(n.getVarDefs());
	}
	public void addDecl(GlobalDecl d)
	{
		if (d == null)
			return;
		decls.add(d);
	}
	public void addVar(Var v)
	{
		if (v == null)
			return;
		vars.add(v);
	}
	
	public void combineWith(Plan p)
	{
		if (p == null)
			return;
		
		if (p.getNodes() != null)
			nodes.addAll(p.getNodes());
		if (p.getDecls() != null)
			decls.addAll(p.getDecls());
		if (p.getVars() != null)
			vars.addAll(p.getVars());
	}
	
	public Var findVar(String id)
	{
		for (Var v : vars)
		{
			if (v.getID().equals(id))
				return v;
		}
		return null;
	}
	
	public Var findVarInScope(String id, Node n)
	{
		if (n == null)
			return null;
		
		return n.findVarInScope(id);
	}

	public Node findNode(String id)
	{
		for (Node n : nodes)
		{
			if (n.getID().equals(id))
				return n;
		}
		return null;
	}
	
	public GlobalDecl findDecl(String id)
	{
		for (GlobalDecl c : decls)
		{
			if (c.getID().equals(id))
				return c;
		}
		return null;
	}
}
