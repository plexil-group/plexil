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
import java.util.Vector;

public class XmlReader {
	public static final String MainRootText = "PlexilPlan";
	public static final String NodeRootText = "Node";
	
	private IXMLElement declXml = null;
	private IXMLElement nodeXml = null;
	private Plan plan = null;
	
	public GlobalDeclList getDecls() { return plan.getDecls(); }
	public Plan getPlan() { return plan; }

	public void readPlan(IXMLElement p)
	{
		if (!p.getName().equals(MainRootText))
		{
			System.out.println("Not a Plexil root node. Exiting.");
			return;
		}
		
		splitXml(p);
		
		PlanReader pr = new PlanReader();
		plan = pr.buildPlan(nodeXml);

		DeclReader dr = new DeclReader();
		plan.setDecls(dr.extractCalls(declXml));

		return;

	}
	
	@SuppressWarnings("unchecked")
	private void splitXml(IXMLElement p)
	{
		Vector<IXMLElement> nodes = p.getChildren();
		for (IXMLElement currentNode : nodes)
		{
			String text = currentNode.getName();
			if(text.equals(DeclReader.DeclarationRootText))
				declXml = currentNode;
			if(text.equals(PlanReader.NodeRootText))
				nodeXml = currentNode;
		}
	}
	
	public void printDeclXml()
	{
		System.out.println("Declarations:");
		printNodeTree(declXml);
	}

	public void printNodeXml()
	{
		System.out.println("Nodes:");
		printNodeTree(nodeXml);
	}
	
	public void printDeclCalls()
	{
		for (GlobalDecl decl : plan.getDecls())
			decl.print();
	}

	public void printNodeCalls()
	{
		for (Node node : plan.getNodes())
			node.print();
	}

	private void printNodeTree(IXMLElement node)
	{
		if (node == null)
			return;
		
		System.out.println(node.getName());
		printChildren(node, 1, true);
		System.out.println();
	}
	
	@SuppressWarnings("unchecked")
	private void printChildren(IXMLElement nodes, int indent, boolean recurse)
	{
		Vector<IXMLElement> nodeList = nodes.getChildren();
		for (IXMLElement currentNode : nodeList)
		{
			String text = currentNode.getName();
			for (int j = 0; j < indent; j++)
				System.out.print("  ");
			System.out.println(text);
			if (recurse)
				printChildren(currentNode, indent + 1, recurse);
		}
	}
}
