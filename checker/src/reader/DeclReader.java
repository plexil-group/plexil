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

import model.*;
import model.GlobalDecl.CallType;
import model.Var.VarType;
import model.Var.VarMod;
import net.n3.nanoxml.*;

import java.util.Vector;

public class DeclReader {
	public static final String DeclarationRootText = "GlobalDeclarations";
	public static final String CommandDeclText = "Command";
	public static final String FunctionDeclText = "FunctionDeclaration";
	public static final String LookupDeclText = "StateDeclaration";
	public static final String LibraryNodeDeclText = "LibraryNodeDeclaration";
	public static final String NameDeclarationText = "Name";
	public static final String TypeDeclarationText = "Type";
	public static final String ParameterDeclarationText = "Parameter";
	public static final String ReturnDeclarationText = "Return";
	public static final String InterfaceDeclarationText = "Interface";
	public static final String InDeclarationText = "In";
	public static final String InOutDeclarationText = "InOut";

	public GlobalDeclList extractCalls(IXMLElement xml) {
		GlobalDeclList decls = new GlobalDeclList();
		
		IXMLElement declXml = findXmlByTerm(xml, DeclarationRootText);
		if (declXml == null)
			return decls;
		
		for (IXMLElement child : getChildren(declXml))
		{
			GlobalDecl d = convertXmlToDec(child);
			if (d != null)
				decls.add(d);
		}
		return decls;
	}

	private GlobalDecl convertXmlToDec(IXMLElement xml)
	{
		CallType declType = CallType.Command;
		String id = "";
		VarList params = new VarList();
		VarList rets = new VarList();
		
		if (xml.getName().equals(CommandDeclText))
			declType = CallType.Command;
		if (xml.getName().equals(FunctionDeclText))
			declType = CallType.Function;
		if (xml.getName().equals(LookupDeclText))
			declType = CallType.Lookup;
		if (xml.getName().equals(LibraryNodeDeclText))
			declType = CallType.LibraryCall;
		
		if (!xml.hasChildren())
			return null;
		
		for (IXMLElement child : getChildren(xml))
		{
			String text = child.getName();
			if (text.equals(ParameterDeclarationText))
				params.add(convertXmlToVar(child));
			else if (text.equals(ReturnDeclarationText))
				rets.add(convertXmlToVar(child));
			else if (text.equals(NameDeclarationText))
				id = child.getContent();
			else if (text.equals(InterfaceDeclarationText))
				params.addAll(convertXmlToInterfaceVars(child));
		}

		return new GlobalDecl(declType, id, params, rets);
	}
	
	private Var convertXmlToVar(IXMLElement xml)
	{
		if (xml == null)
			return null;
		
		VarType type = null;
		
		IXMLElement typeElt = xml.getFirstChildNamed(TypeDeclarationText);
		if (typeElt != null) {
			type = stringToVarType(typeElt.getContent());
		}

		IXMLElement nameElt = xml.getFirstChildNamed(NameDeclarationText);
		if (nameElt != null)
			return new Var(nameElt.getContent(), type, VarMod.None);
		return new Var(type, VarMod.None);
	}
	
	public IXMLElement findXmlByTerm(IXMLElement xml, String term)
	{
		if (xml == null)
			return null;
		
		if (xml.getName().equals(term))
			return xml;
		
		for (IXMLElement child : getChildren(xml))
		{
			IXMLElement result = findXmlByTerm(child, term);
			if (result != null)
				return result;
		}
		return null;
	}
	
	private VarList convertXmlToInterfaceVars(IXMLElement xml)
	{
		if (xml == null)
			return null;
		
		VarList list = new VarList();
		for (IXMLElement varXml : getChildren(xml))
		{
			if (varXml.getName() == null)
				continue;
			
			VarMod mod = VarMod.None;
			if (varXml.getName().equals(InDeclarationText))
				mod = VarMod.In;
			else if (varXml.getName().equals(InOutDeclarationText))
				mod = VarMod.InOut;
			
			// If there is no delcaration, there's no Var to create
			if (!varXml.hasChildren() || !varXml.getChildAtIndex(0).hasChildren())
				continue;
			
			String name = null;
			VarType type = null;
			// Skip over an XML level that holds DeclareVariable, hence .getChildAtIndex(0)
			for (IXMLElement child : getChildren(varXml.getChildAtIndex(0)))
			{
				if (child.getName().equals(NameDeclarationText))
					name = child.getContent();
				if (child.getName().equals(TypeDeclarationText))
				{
					String typeStr = child.getContent();
					if (typeStr != null) {
						type = stringToVarType(typeStr);
						// *** TEMPORARY ***
						if (type == null) {
							System.out.println(" In \"" + child.toString() + "\"");
						}
					}
				}
			}
			
			if (name != null)
				list.add(new Var(name, type, mod));
			else
				list.add(new Var(type, mod));
		}
		return list;
	}

	private VarType stringToVarType(String s)
	{
		if (s.equals("B") || s.equals("Boolean"))
			return VarType.B;
		if (s.equals("I") || s.equals("Integer"))
			return VarType.I;
		if (s.equals("R") || s.equals("Real"))
			return VarType.R;
		if (s.equals("S") || s.equals("String"))
			return VarType.S;

		System.out.println("Could not determine type of variable " + s);
		return null;
	}
	
	// IXMLElement.getChildren() only returns Vector. Having a typed vector is nicer, so we type cast here.
	@SuppressWarnings("unchecked")
	private Vector<IXMLElement> getChildren(IXMLElement elem)
	{
		Vector<IXMLElement> children = elem.getChildren();
		return children;
	}
}
