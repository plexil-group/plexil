// Copyright (c) 2006-2008, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


// AST parser for PLEXIL

header {
package plexil;

import java.text.NumberFormat;
import java.util.Iterator;
import java.util.Vector;

import net.n3.nanoxml.*;

import plexil.PlexilASTNode;
import plexil.PlexilDataType;
import plexil.PlexilGlobalDeclaration;
import plexil.PlexilNameType;

}

class PlexilTreeParser extends TreeParser;

options {
  importVocab = Plexil; // call its vocabulary "plexil"
}

{
    static final int PLEXIL_MAJOR_RELEASE = 1;
    static final int PLEXIL_MINOR_RELEASE = 0;
    static final int PLEXIL_PARSER_RELEASE = 0;

    PlexilParserState state = null;
    PlexilNodeContext context = PlexilGlobalContext.getGlobalContext();
    NumberFormat numberFormat = NumberFormat.getInstance();

    public PlexilTreeParser(PlexilParserState inState)
    {
        super();
        state = inState;
    }

    // For the convenience of the plan editor ONLY!!

    public void setContext(PlexilNodeContext ctxt)
    {
        context = ctxt;
    }

	protected void copyPosition(IXMLElement object, AST n) throws ClassCastException {
		if(!(n instanceof PlexilASTNode))
			throw new ClassCastException("cannot copy position from AST!");
		PlexilASTNode node = (PlexilASTNode) n;
		if ((node!=null) && (object!=null)) {
			if (node.getLine()>=0) {
				object.setAttribute(PlexilXmlStrings.x_line,Integer.toString(node.getLine()));
			}
			if (node.getColumn()>=0) {
			  object.setAttribute(PlexilXmlStrings.x_column,Integer.toString(node.getColumn()));
			}
		}
	}

	protected void copyFilename(IXMLElement object, AST n) throws ClassCastException 
    {
		if(!(n instanceof PlexilASTNode))
			throw new ClassCastException("cannot copy filename from AST!");
		PlexilASTNode node = (PlexilASTNode) n;
		if ((node!=null) && (object!=null)) {
			if (node.getColumn()>=0) {
			  object.setAttribute(PlexilXmlStrings.x_filename,node.getFilename());
			}
		}
    }

    private SemanticException createSemanticException(String message, 
                                                      antlr.collections.AST location)
    {
      PlexilASTNode plexLoc = null;
      if (location != null)
        {
          plexLoc = (PlexilASTNode) location;
        }
      if (plexLoc == null)
        return new SemanticException(message);
      else
        return new SemanticException(message,
                                     plexLoc.getFilename(),
                                     plexLoc.getLine(),
                                     plexLoc.getColumn());
    }

    private int parseInteger(AST the_int)
      throws SemanticException
    {
      int result = 0;
      try
      {
        result = numberFormat.parse(the_int.getText()).intValue();
      }
      catch (java.text.ParseException e)
      {
        throw createSemanticException("Internal error: \""
                                      + the_int.getText()
                                      + "\" is not parsable as an integer",
                                      the_int);
      }
      return result;
    }

}

//
// TREE GRAMMAR BEGINS HERE
//

plexilPlan[IXMLElement plexilPlan] :
   #(PLEXIL (globalDeclarations[plexilPlan])?  (node[plexilPlan])* )
   {
     plexilPlan.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
     // *** subject to change! ***
     plexilPlan.setAttribute("xsi:noNamespaceSchemaLocation", "plexil.xsd");
   }
 ;


// *** need to output declarations in correct order:
//  command, lookup, function, time scaling

globalDeclarations[IXMLElement parent]
{ IXMLElement globals = new XMLElement("GlobalDeclarations"); }
 :
    #(GLOBAL_DECLARATIONS ( globalDeclaration[globals] )+ )
   { 
     // extract declarations from global context, 
     // output in correct order
     Vector commandDecls = new Vector();
     Vector lookupDecls = new Vector();
     Vector functionDecls = new Vector();
     PlexilGlobalContext.getGlobalContext().getGlobalDeclarations(commandDecls,
                                                                  lookupDecls,
                                                                  functionDecls);
     for (Iterator commandIt = commandDecls.iterator(); commandIt.hasNext(); )
       {
         // process one command declaration
         IXMLElement commandXml = 
                ((PlexilGlobalDeclaration) commandIt.next()).getXml();
         globals.addChild(commandXml);
       }
     for (Iterator lookupIt = lookupDecls.iterator(); lookupIt.hasNext(); )
       {
         // process one lookup declaration
         IXMLElement lookupXml = 
                ((PlexilGlobalDeclaration) lookupIt.next()).getXml();
         globals.addChild(lookupXml);
       }
     for (Iterator functionIt = functionDecls.iterator(); functionIt.hasNext(); )
       {
         // process one function declaration
         IXMLElement functionXml = 
                ((PlexilGlobalDeclaration) functionIt.next()).getXml();
         globals.addChild(functionXml);
       }

     parent.addChild(globals); 
   } ;
       
globalDeclaration[IXMLElement parent] :
        commandDeclaration[parent]
        | functionDeclaration[parent]
        | lookupDeclaration[parent]
        | libraryNodeDeclaration ;

//
// N.B.:
// Declarations are input into the global context by the parser (see plexil.g),
// and are output at globalDeclarations above.  So no need to do anything here.
//

commandDeclaration[IXMLElement parent]
{ IXMLElement decl = new XMLElement("CommandDeclaration"); }
 :
        #(COMMAND_KYWD
            cn:commandName[decl]
            (ret:returnsSpec[decl])?
            (parms:paramsSpec[decl])?
            )
     { 
     } ;

functionDeclaration[IXMLElement parent]
{ IXMLElement decl = new XMLElement("FunctionDeclaration"); }
 : 
        #(FUNCTION_KYWD
            fn:functionName[decl]
            (ret:returnsSpec[decl])?
            (parms:paramsSpec[decl])?
            )
    { 
    } ;

lookupDeclaration[IXMLElement parent]
{ IXMLElement decl = new XMLElement("StateDeclaration"); }
 : 
    #(LOOKUP_KYWD 
       sn:stateName[decl]
       ret:returnsSpec[decl] 
       ( parms:paramsSpec[decl] )? 
     )
    { 
    } ;

paramsSpec[IXMLElement parent] :
        #(PARAMETER_DECLARATIONS ( paramSpec[parent] )* ) ;

paramSpec[IXMLElement parent] :
        #(PARAMETER tn:typeName (pn:paramName)? )
        {
            IXMLElement paramXml = new XMLElement("Parameter");
            paramXml.setAttribute("Type", #tn.getText());
            if (#pn != null)
              paramXml.setContent(#pn.getText());
            parent.addChild(paramXml);
        } ;

returnsSpec[IXMLElement parent] :
        #(RETURNS_KYWD ( returnSpec[parent] )* ) ;

returnSpec[IXMLElement parent] :
        #(RETURN_VALUE tn:typeName (pn:paramName)? )
        {
            IXMLElement rtnXml = new XMLElement("Return");
            rtnXml.setAttribute("Type", #tn.getText());
            if (#pn != null)
              rtnXml.setContent(#pn.getText());
            parent.addChild(rtnXml);
        } ;

typeName :
        BLOB_KYWD
        | BOOLEAN_KYWD
        | INTEGER_KYWD
        | REAL_KYWD
        | STRING_KYWD
        | TIME_KYWD
        | NO_TYPE ;

paramName : NCName ;

// No need to do anything, surface parser has already handled this declaration.
libraryNodeDeclaration : #(LIBRARY_NODE_KYWD NodeName (libraryInterfaceSpec)? ) ;

libraryInterfaceSpec : #(PARAMETER_DECLARATIONS (libraryParamSpec)* ) ;

libraryParamSpec: #(PARAMETER typeName paramName) ;

// *** N.B. xnode variable is an XMLElement so we can use the insertChild() method,
// which isn't supported on IXMLElement.

// *** N.B. Node AST contains the context from the surface parser.

// *** N.B. Node ID is auto-generated by surface parser

node[IXMLElement parent] 
{
  XMLElement xnode = new XMLElement("Node");
  XMLElement xmlResourceList = new XMLElement("ResourceList");
  // initialize node type attribute to empty
  xnode.setAttribute("NodeType", "Empty");
}
 :
 #(n:NODE { 
            parent.addChild(xnode);
            context = ((NodeASTNode) #n).getContext();
          }
          #(NODE_ID id:NodeName)
           { if (#id != null)
             {
                  IXMLElement nodeId = new XMLElement("NodeId");
                  xnode.addChild(nodeId);
                  nodeId.setContent(#id.getText());
             }
           }        
          (comment[xnode])?
          (nodeDeclaration[xnode])*
          (nodeAttribute[xnode, xmlResourceList])* // build resource list here...
          (nodeBody[xnode, xmlResourceList])?      // consume it here
          )
  {
    // Add all variable declarations to node
    Vector localVars = new Vector();
    Vector inVars = new Vector();
    Vector inOutVars = new Vector();
    context.getNodeVariables(localVars, inVars, inOutVars);

    if (!localVars.isEmpty())
      {
        IXMLElement varDeclXml = xnode.getFirstChildNamed("VariableDeclarations");
        if (varDeclXml == null)
          {
            varDeclXml = new XMLElement("VariableDeclarations");
            xnode.insertChild(varDeclXml, 0);
          }
        if (!localVars.isEmpty())
          {                
            for (Iterator it = localVars.iterator(); it.hasNext(); )
              {
                PlexilVariableName var = (PlexilVariableName) it.next();
                IXMLElement xdecl = var.makeVariableDeclarationElement();
                varDeclXml.addChild(xdecl);
              }
          }
      }

    if (!inVars.isEmpty() || !inOutVars.isEmpty())
      {
        IXMLElement intfXml = xnode.getFirstChildNamed("Interface");
        if (intfXml == null)
          {
            intfXml = new XMLElement("Interface");
            xnode.insertChild(intfXml, 0);
          }
        if (!inVars.isEmpty())
          {                
            IXMLElement inXml = new XMLElement("In");                    
            for (Iterator it = inVars.iterator(); it.hasNext(); )
              {
                IXMLElement xdecl =
                  ((PlexilVariableName) it.next()).makeVariableDeclarationElement();
                inXml.addChild(xdecl);
              }
            intfXml.addChild(inXml);
          }
        if (!inOutVars.isEmpty())
          {                
            IXMLElement inOutXml = new XMLElement("InOut");
            for (Iterator it = inOutVars.iterator(); it.hasNext(); )
              {
                IXMLElement xdecl =
                  ((PlexilVariableName) it.next()).makeVariableDeclarationElement();
                inOutXml.addChild(xdecl);
              }
            intfXml.addChild(inOutXml);
          }
      }
    
    // pop out to parent context on exit
    context = context.getParentContext();
  }
 ;

comment[IXMLElement parent]
{ IXMLElement cmnt = new XMLElement("Comment"); }
 :
 #(COMMENT_KYWD { parent.addChild(cmnt); }
    str:STRING { cmnt.setContent(#str.getText()); }
  ) ;

nodeDeclaration[IXMLElement node] :
        interfaceDeclaration
        | variableDeclaration ;

nodeAttribute[IXMLElement node, XMLElement xmlResourceList] 
    : startCondition[node]
    | repeatCondition[node]
    | skipCondition[node]
    | preCondition[node]
    | postCondition[node]
    | invariantCondition[node]
    | endCondition[node]
    | priority[node]
    | resource[node, xmlResourceList]
    | resourcePriority
    | permissions[node];

startCondition[IXMLElement node]
{ IXMLElement xsc = new XMLElement("StartCondition"); }
 :
   #(START_CONDITION_KYWD booleanExpression[xsc])
   {
     node.addChild(xsc);
   }
 ;

repeatCondition[IXMLElement node]
{ IXMLElement xrc = new XMLElement("RepeatCondition"); }
 :
   #(REPEAT_CONDITION_KYWD booleanExpression[xrc])
   {
     node.addChild(xrc);
   }
 ;

skipCondition[IXMLElement node]
{ IXMLElement xsc = new XMLElement("SkipCondition"); }
 :
   #(SKIP_CONDITION_KYWD booleanExpression[xsc])
   {
     node.addChild(xsc);
   }
 ;

preCondition[IXMLElement node]
{ IXMLElement xpc = new XMLElement("PreCondition"); }
 : 
   #(PRE_CONDITION_KYWD booleanExpression[xpc])
   {
     node.addChild(xpc);
   }
 ;

postCondition[IXMLElement node]
{ IXMLElement xpc = new XMLElement("PostCondition"); }
 : 
   #(POST_CONDITION_KYWD booleanExpression[xpc])
   {
     node.addChild(xpc);
   }
 ;

invariantCondition[IXMLElement node]
{ IXMLElement xic = new XMLElement("InvariantCondition"); }
 : 
   #(INVARIANT_CONDITION_KYWD booleanExpression[xic])
   {
     node.addChild(xic);
   }
 ;

endCondition[IXMLElement node]
{ IXMLElement xec = new XMLElement("EndCondition"); }
 : 
   #(END_CONDITION_KYWD booleanExpression[xec])
   {
     node.addChild(xec);
   }
 ;

// This form should not appear in the output -
// it is for internal use only

resourcePriority
{ IXMLElement xrp = new XMLElement("ResourcePriority"); }
 :
 #(RESOURCE_PRIORITY_KYWD numericExpression[xrp])
 {
   context.setResourcePriorityXML(xrp);
 }
;


// The surface parser guarantees there will be no more than one of each option

resource[IXMLElement node, XMLElement resourceList]
{   
    IXMLElement xmlResource = new XMLElement("Resource");
    IXMLElement xmlName = new XMLElement("ResourceName");
    IXMLElement xmlPrio = new XMLElement("ResourcePriority");
    IXMLElement xmlLowerBound = new XMLElement("ResourceLowerBound");
    IXMLElement xmlUpperBound = new XMLElement("ResourceUpperBound");
    IXMLElement xmlReleaseAtTerm = new XMLElement("ResourceReleaseAtTermination");
}
 : 
        #(RESOURCE_KYWD 
            stringExpression[xmlName]
            { xmlResource.addChild(xmlName); }

            ( ( PRIORITY_KYWD numericExpression[xmlPrio]
                  { xmlResource.addChild(xmlPrio); } )
              | ( LOWER_BOUND_KYWD numericExpression[xmlLowerBound]
                  { xmlResource.addChild(xmlLowerBound); } ) 
              | ( UPPER_BOUND_KYWD numericExpression[xmlUpperBound]
                  { xmlResource.addChild(xmlUpperBound); } ) 
              | ( RELEASE_AT_TERM_KYWD booleanExpression[xmlReleaseAtTerm]
                  { xmlResource.addChild(xmlReleaseAtTerm); } )
            )*
        )
        {
          resourceList.addChild(xmlResource);
        }
    ;

priority[IXMLElement node]
{ IXMLElement xprio = new XMLElement("Priority"); }
 : 
   #(PRIORITY_KYWD i:nonNegativeInteger)
   {
     xprio.setContent(#i.getText());
     node.addChild(xprio);
   }
   ;

// *** need to add test for non-negative
nonNegativeInteger : INT ;

permissions[IXMLElement node]
{ IXMLElement xperm = new XMLElement("Permissions"); }
 :
   #(PERMISSIONS_KYWD s:STRING)
   {
     xperm.setContent(#s.getText());
     node.addChild(xperm);
   }
   ;

// XML construction no longer done here, but at node exit time
interfaceDeclaration : 
   in | inOut ;

// *** N.B.  Interface variables now come from the surface parser with the context,
// no need to add them here.

// XML construction no longer done here, but at node exit time
in : #(IN_KYWD (tn:typeName)? (NCName)* )
  {
    AST var = #in.getFirstChild();
    PlexilDataType typ = null;
    if (#tn != null)
    {
      var = var.getNextSibling(); // skip over type
      typ = PlexilDataType.findByName(#tn.getText());
      if (typ == null)
      {
        throw createSemanticException("Internal error: \""
                                      + #tn.getText()
                                      + "\" is an unknown data type in an In statement",
                                      #tn);
      }
    }
    if (context.isLibraryNode())
    {
      if (#tn == null)
        {
          throw new SemanticException("Internal error: In statement has no type in library node!");
        }
    }
    while (var != null)
    {
      PlexilVariableName varName = context.findVariable(var.getText());
      if (varName == null)
      {
        throw createSemanticException("Internal error: In variable \""
                                      + var.getText()
                                      + "\" not found in context",
                                      var);
      }
      if (varName.isLocal())
      {
        throw createSemanticException("Internal error: In variable \""
                                      + var.getText()
                                      + "\" is not an interface variable",
                                      var);
      }
      if (varName.isAssignable())
      {
        throw createSemanticException("Internal error: In variable \""
                                      + var.getText()
                                      + "\" is assignable",
                                      var);
      }
      var = var.getNextSibling();
    }
  }
;

// XML construction no longer done here, but at node exit time
inOut : #(IN_OUT_KYWD (tn:typeName)? (NCName)* )
  {
    AST var = #inOut.getFirstChild();
    PlexilDataType typ = null;
    if (#tn != null)
    {
      var = var.getNextSibling(); // skip over type
      typ = PlexilDataType.findByName(#tn.getText());
      if (typ == null)
      {
        throw createSemanticException("Internal error: \""
                                      + #tn.getText()
                                      + "\" is an unknown data type in an InOut statement",
                                      #tn);
      }
    }
    if (context.isLibraryNode())
    {
      if (#tn == null)
        {
          throw new SemanticException("Internal error: InOut statement has no type in library node!");
        }
    }
    while (var != null)
    {
      PlexilVariableName varName = context.findVariable(var.getText());
      if (varName == null)
      {
        throw createSemanticException("Internal error: InOut variable \""
                                      + var.getText()
                                      + "\" not found in context",
                                      var);
      }
      if (varName.isLocal())
      {
        throw createSemanticException("Internal error: InOut variable \""
                                      + var.getText()
                                      + "\" is not an interface variable",
                                      var);
      }
      if (!varName.isAssignable())
      {
        throw createSemanticException("Internal error: InOut variable \""
                                      + var.getText()
                                      + "\" is not assignable",
                                      var);
      }
      var = var.getNextSibling();
    }
  }
 ;


//
// Variable Declarations
//

// Similar to above -- construction done at node exit

variableDeclaration :
 #(VARIABLE_DECLARATION
   (booleanVariableDeclaration
    | integerVariableDeclaration
    | realVariableDeclaration
    | stringVariableDeclaration
    | blobVariableDeclaration
    | timeVariableDeclaration
    | booleanArrayDeclaration
    | integerArrayDeclaration
    | realArrayDeclaration
    | stringArrayDeclaration
    | blobArrayDeclaration
    | timeArrayDeclaration
   )* 
  )
 ;

//
// Variable declarations
//
// No need to add variables to context - are already there from surface parser
// However, we need to add the initial values because the surface parser
// doesn't know how to generate the XML for them.
//

booleanVariableDeclaration :
 #(BOOLEAN_KYWD vn:variableName (iv:booleanValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.BOOLEAN_TYPE)
     throw createSemanticException("Internal error: Boolean variable named \""
                                   + #vn.getText() + "\" is not Boolean!",
                                   #vn);
   if (#iv != null)
   {
     var.setInitialValue(#iv);
   }
 }
 ;

booleanArrayDeclaration :
 #(BOOLEAN_ARRAY avn:variableName dim:nonNegativeInteger (aiv:booleanArrayInitialValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.BOOLEAN_ARRAY_TYPE)
     throw createSemanticException("Internal error: Boolean array variable named \""
                                   + #avn.getText() + "\" is not a Boolean array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(#aiv);
   }
 }
 ;

booleanArrayInitialValue :
 booleanValue |
 #(BOOLEAN_ARRAY_LITERAL ( booleanValue )* ) ;

integerVariableDeclaration :
 #(INTEGER_KYWD vn:variableName (iv:integerValue)? )
 {
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.INTEGER_TYPE)
     throw createSemanticException("Internal error: Integer variable named \""
                                   + #vn.getText() + "\" is not integer!",
                                   #vn);
   if (#iv != null)
   {
     var.setInitialValue(#iv);
   }
 }
 ; 

integerArrayDeclaration :
 #(INTEGER_ARRAY avn:variableName dim:nonNegativeInteger (aiv:integerArrayInitialValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.INTEGER_ARRAY_TYPE)
     throw createSemanticException("Internal error: Integer array variable named \""
                                   + #avn.getText() + "\" is not an integer array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(#aiv);
   }
 }
 ;

integerArrayInitialValue :
 integerValue |
 #(INTEGER_ARRAY_LITERAL ( integerValue )* ) ;

// *** needs work!

blobVariableDeclaration :
 #(BLOB_KYWD vn:variableName (pt:pointsTo)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.BLOB_TYPE)
     throw createSemanticException("Internal error: BLOB variable named \""
                                   + #vn.getText() + "\" is not BLOB!",
                                   #vn);
   if (#pt != null)
   {
     var.setInitialValue(((PlexilASTNode) #pt).getXmlElement());
   }
 }
 ;


blobArrayDeclaration :
 #(BLOB_ARRAY avn:variableName dim:nonNegativeInteger (aiv:pointsTo)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.BLOB_ARRAY_TYPE)
     throw createSemanticException("Internal error: BLOB array variable named \""
                                   + #avn.getText() + "\" is not a BLOB array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(((PlexilASTNode) #aiv).getXmlElement());
   }
 }
 ;

pointsTo :
   #(POINTS_TO externalStructName (pointerInitialValue)? ) ;

externalStructName : 
   NCName ;

pointerInitialValue :
   INT ;

realVariableDeclaration :
 #(REAL_KYWD vn:variableName (iv:realValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.REAL_TYPE)
     throw createSemanticException("Internal error: Real variable named \""
                                   + #vn.getText() + "\" is not Real!",
                                   #vn);
   if (#iv != null)
   {
     var.setInitialValue(#iv);
   }
 }
 ;

realArrayDeclaration :
 #(REAL_ARRAY avn:variableName dim:nonNegativeInteger (aiv:realArrayInitialValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.REAL_ARRAY_TYPE)
     throw createSemanticException("Internal error: Real array variable named \""
                                   + #avn.getText() + "\" is not a real array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(#aiv);
   }
 }
 ;

realArrayInitialValue :
 realValue |
 #(REAL_ARRAY_LITERAL ( realValue )* ) ;

stringVariableDeclaration :
 #(STRING_KYWD vn:variableName (iv:stringValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.STRING_TYPE)
     throw createSemanticException("Internal error: String variable named \""
                                   + #vn.getText() + "\" is not String!",
                                   #vn);
   if (#iv != null)
   {
     var.setInitialValue(#iv);
   }
 }
 ;

stringArrayDeclaration :
 #(STRING_ARRAY avn:variableName dim:nonNegativeInteger (aiv:stringArrayInitialValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.STRING_ARRAY_TYPE)
     throw createSemanticException("Internal error: String array variable named \""
                                   + #avn.getText() + "\" is not a string array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(#aiv);
   }
 }
 ; 

stringArrayInitialValue :
 stringValue |
 #(STRING_ARRAY_LITERAL ( stringValue )* ) ;

timeVariableDeclaration :
 #(TIME_KYWD vn:variableName (iv:timeValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#vn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #vn.getText() + "\"",
                                   #vn);
   if (var.getVariableType() != PlexilDataType.TIME_TYPE)
     throw createSemanticException("Internal error: Time variable named \""
                                   + #vn.getText() + "\" is not Time!",
                                   #vn);
   if (#iv != null)
   {
     var.setInitialValue(#iv);
   }
 }
 ;

timeArrayDeclaration :
 #(TIME_ARRAY avn:variableName dim:nonNegativeInteger (aiv:timeArrayInitialValue)? )
 { 
   PlexilVariableName var = context.findLocalVariable(#avn.getText());
   if (var == null)
     throw createSemanticException("Internal error: context contains no variable named \""
                                   + #avn.getText() + "\"",
                                   #avn);
   if (var.getVariableType() != PlexilDataType.TIME_ARRAY_TYPE)
     throw createSemanticException("Internal error: Time array variable named \""
                                   + #avn.getText() + "\" is not a Time array!",
                                   #avn);
   if (#aiv != null)
   {
     var.setInitialValue(#aiv);
   }
 }
 ;

timeArrayInitialValue :
 timeValue |
 #(TIME_ARRAY_LITERAL ( timeValue )* ) ;

// first integerValue is units, 2nd (if present) is subunits
timeValue :
   #(TIME_VALUE integerValue (integerValue)? ) ;

// Do semantic checks elsewhere
variableName :
   NCName ;

// Surface parser figures this part out for us
variable[IXMLElement parent] :
 VARIABLE
 {
   parent.addChild(((VariableASTNode) #variable).getXMLElement());
 }
 ;

// must also add semantic predicate here because these are used in other contexts

booleanVariable[IXMLElement parent] :
 n:VARIABLE { context.isBooleanVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #booleanVariable).getXMLElement());
 }
 ;

integerVariable[IXMLElement parent] :
 n:VARIABLE { context.isIntegerVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #integerVariable).getXMLElement());
 }
 ;

blobVariable[IXMLElement parent] :
 n:VARIABLE { context.isBlobVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #blobVariable).getXMLElement());
 }
 ;

realVariable[IXMLElement parent] :
 n:VARIABLE { context.isRealVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #realVariable).getXMLElement());
 }
 ;

stringVariable[IXMLElement parent] :
 n:VARIABLE { context.isStringVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #stringVariable).getXMLElement());
 }
 ;

timeVariable[IXMLElement parent] :
 n:VARIABLE { context.isTimeVariableName(#n.getText()) }?
 {
   parent.addChild(((VariableASTNode) #timeVariable).getXMLElement());
 }
 ;

arrayVariable :
 n:VARIABLE { context.isArrayVariableName(#n.getText()) }? ;


// Have to deal with index expression explicitly for now

arrayReference[IXMLElement parent]
{
  IXMLElement xdx = new XMLElement("Index");
}
 :
  #(ARRAY_REF arrayVariable idx:numericExpression[xdx])
  {
    IXMLElement aref = ((PlexilASTNode) #arrayReference).getXmlElement();
    aref.addChild(xdx);
    parent.addChild(aref);
  }
  ;

booleanArrayReference[IXMLElement parent] : arrayReference[parent] ;

integerArrayReference[IXMLElement parent] : arrayReference[parent] ;

blobArrayReference[IXMLElement parent] : arrayReference[parent] ;

realArrayReference[IXMLElement parent] : arrayReference[parent] ;

stringArrayReference[IXMLElement parent] : arrayReference[parent] ;

timeArrayReference[IXMLElement parent] : arrayReference[parent] ;

nodeBody[IXMLElement node, IXMLElement xmlResourceList]
{
  IXMLElement xNodeBody = new XMLElement("NodeBody");
}
 : 
   ( nodeList[node, xNodeBody]
     | command[node, xNodeBody, xmlResourceList]
     | functionCall[node, xNodeBody]
     | assignment[node, xNodeBody]
     | update[node, xNodeBody]
     | request[node, xNodeBody]
     | libraryCall[node, xNodeBody]
   )
   {
     node.addChild(xNodeBody); 
   }
   ;

nodeList[IXMLElement node, IXMLElement nodeBody] 
{ IXMLElement nlist = new XMLElement("NodeList"); }
 :
   #(NODE_LIST_KYWD (node[nlist])* )
   {
     node.setAttribute("NodeType", "NodeList");
     nodeBody.addChild(nlist);
   }
   ;


// *** add variable assignability check? (probably move to surface parser)

command[IXMLElement node, IXMLElement nodeBody, IXMLElement resourceList]
{ 
  IXMLElement cmd = new XMLElement("Command");
  if (resourceList.getChildrenCount() > 0)
  {
    // supply default priority
    IXMLElement priority = context.getResourcePriorityXML();
    if (priority != null)
    {
      // add priority to resources
      for (int i = 0; i < resourceList.getChildrenCount(); ++i)
      {
         IXMLElement resource = resourceList.getChildAtIndex(i);
         if (resource.getFirstChildNamed("ResourcePriority") == null)
           resource.addChild(priority);
      }
    }
    cmd.addChild(resourceList);
  }
}
 : 
  (
   ( #(COMMAND_KYWD ARRAY_REF) )=>
   #(COMMAND_KYWD arrayReference[cmd] EQUALS commandName[cmd] (argumentList[cmd])? )
   |
   ( #(COMMAND_KYWD VARIABLE) )=>
   #(COMMAND_KYWD variable[cmd] EQUALS commandName[cmd] (argumentList[cmd])? )
   |
   #(COMMAND_KYWD commandName[cmd] (argumentList[cmd])? )
  )
   {
     node.setAttribute("NodeType", "Command");
     nodeBody.addChild(cmd);
   }
   ;

commandName[IXMLElement parent]
{ IXMLElement cn = new XMLElement("Name"); }
 : NCName
   {
     IXMLElement sv = new XMLElement("StringValue");
     sv.setContent(#commandName.getText());
     cn.addChild(sv);
     parent.addChild(cn);
   }
 ;

argumentList[IXMLElement parent]
{ 
  IXMLElement xargs = new XMLElement("Arguments");
  parent.addChild(xargs);
}
 : 
   #(ARGUMENT_LIST (argument[xargs])+ ) ;

argument[IXMLElement parent] : argValue[parent] | variable[parent] | arrayReference[parent] ;

argValue[IXMLElement parent] :
 ( iv:integerValue { parent.addChild(((PlexilASTNode) #iv).getXmlElement()); }
   | rv:realValue { parent.addChild(((PlexilASTNode) #rv).getXmlElement()); }
   | sv:stringValue { parent.addChild(((PlexilASTNode) #sv).getXmlElement()); }
   | tv:timeValue { parent.addChild(((PlexilASTNode) #tv).getXmlElement()); }
   | bv:booleanValue { parent.addChild(((PlexilASTNode) #bv).getXmlElement()); }
 )
 ;

// *** add assignability check?

functionCall[IXMLElement node, IXMLElement nodeBody]
{ IXMLElement fnCall = new XMLElement("FunctionCall"); }
 : 
   #(FUNCTION_CALL_KYWD
     (variable[fnCall] EQUALS)?
     functionName[fnCall]
     (argumentList[fnCall])? 
    )
   {
     node.setAttribute("NodeType", "FunctionCall");
     nodeBody.addChild(fnCall);
   }
 ;

functionName[IXMLElement parent]
{ IXMLElement fn = new XMLElement("Name"); }
 :
   NCName
   {
     fn.setContent(#functionName.getText());
     parent.addChild(fn);
   }
 ;


// *** To do:
//  - multi-valued lookups
//  - add assignability check? (currently handled in plexil.g)

assignment[IXMLElement node, IXMLElement nodeBody]
{ IXMLElement assign = new XMLElement("Assignment"); }
 : 
   ( booleanAssignment[assign]
     | integerAssignment[assign]
     | realAssignment[assign]
     | stringAssignment[assign]
     | timeAssignment[assign]
     | arrayAssignment[assign]
   )
   {
     node.setAttribute("NodeType", "Assignment");
     nodeBody.addChild(assign);
   }
 ;

booleanAssignment[IXMLElement parent] :
   #(BOOLEAN_ASSIGNMENT
      ( booleanVariable[parent] | booleanArrayReference[parent] )
      bRHS:booleanRHS[parent] )
   ;

booleanRHS[IXMLElement parent]
{ IXMLElement xbRHS = new XMLElement("BooleanRHS"); }
 :
  booleanExpression[xbRHS]
  {
    parent.addChild(xbRHS);
  }
  ;

integerAssignment[IXMLElement parent] :
   #(INTEGER_ASSIGNMENT
      ( integerVariable[parent] | integerArrayReference[parent] )
      numericRHS[parent])
   ;

realAssignment[IXMLElement parent] :
   #(REAL_ASSIGNMENT
       ( realVariable[parent] | realArrayReference[parent] )
       numericRHS[parent])
 ;

numericRHS[IXMLElement parent]
{ IXMLElement xnRHS = new XMLElement("NumericRHS"); }
 : 
   numericExpression[xnRHS] 
   {
     parent.addChild(xnRHS);
   }
;

stringAssignment[IXMLElement parent] :
   #(STRING_ASSIGNMENT 
       ( stringVariable[parent] | stringArrayReference[parent] )
       stringRHS[parent])
 ;

stringRHS[IXMLElement parent]
{ IXMLElement xsRHS = new XMLElement("StringRHS"); }
 : 
   stringExpression[xsRHS]
   {
     parent.addChild(xsRHS);
   }
 ;

timeAssignment[IXMLElement parent] :
   #(TIME_ASSIGNMENT
       ( timeVariable[parent] | timeArrayReference[parent] )
       timeRHS[parent])
 ;

timeRHS[IXMLElement parent]
{ IXMLElement xsRHS = new XMLElement("TimeRHS"); }
 : 
   timeExpression[xsRHS]
   {
     parent.addChild(xsRHS);
   }
 ;

arrayAssignment[IXMLElement parent] :
  #(ARRAY_ASSIGNMENT var:arrayVariable
                     {
                       IXMLElement xname = new XMLElement("ArrayVariable");
                       xname.setContent(#var.getText());
                       parent.addChild(xname); 
                     }
                     arrayRHS[parent])
  ;

// *** For now, use Boolean as dummy type to get around hole in schema
arrayRHS[IXMLElement parent]
{ IXMLElement xsRHS = new XMLElement("BooleanRHS"); }
 :
  arrayExpression[xsRHS]
  {
   parent.addChild(xsRHS);
  }
 ;

// ** needs work ***
arrayExpression[IXMLElement parent] : lookup[parent] ;



//
// Update nodes
//

update[IXMLElement node, IXMLElement nodeBody]
{
  node.setAttribute("NodeType", "Update");
  IXMLElement xupdate = new XMLElement("Update");
  nodeBody.addChild(xupdate);
}
 : #(UPDATE_KYWD ( pair[xupdate] )* ) ;

pair[IXMLElement parent]
{
  IXMLElement xpair = new XMLElement("Pair");
  parent.addChild(xpair);
}
 : name[xpair] ( value[xpair] | variable[xpair] | arrayReference[xpair] | lookupNow[xpair] ) ;

name[IXMLElement parent]
{
  IXMLElement xname = new XMLElement("Name");
  parent.addChild(xname);
}
 : NCName
 { xname.setContent(#name.getText()); } ;

value[IXMLElement parent] :
  ( iv:integerValue { parent.addChild(((PlexilASTNode) #iv).getXmlElement()); }
    | rv:realValue { parent.addChild(((PlexilASTNode) #rv).getXmlElement()); }
    | bv:booleanValue { parent.addChild(((PlexilASTNode) #bv).getXmlElement()); }
    | sv:stringValue { parent.addChild(((PlexilASTNode) #sv).getXmlElement()); }
    | tv:timeValue { parent.addChild(((PlexilASTNode) #tv).getXmlElement()); }
  )
  ; 

//
// Request nodes
//

request[IXMLElement node, IXMLElement nodeBody]
{
  node.setAttribute("NodeType", "Request");
  IXMLElement xrequest = new XMLElement("Request");
  nodeBody.addChild(xrequest);
}
 : #(REQUEST_KYWD nodeNameRef[xrequest] ( pair[xrequest] )* ) ;


//
// Library Call nodes
//

// *** libInterface is temporarily optional ***

libraryCall[IXMLElement node, IXMLElement nodeBody]
{
  node.setAttribute("NodeType", "LibraryNodeCall");
  IXMLElement xlibCall = new XMLElement("LibraryNodeCall");
  nodeBody.addChild(xlibCall);
}
 : #(LIBRARY_CALL_KYWD libraryNodeIdRef[xlibCall] ( aliasSpecs[xlibCall] )? )
 ;

aliasSpecs[IXMLElement parent]
 : #(ALIASES ( constAlias[parent] | varAlias[parent] )* ) ;
  
constAlias[IXMLElement parent]
{
  IXMLElement xalias = new XMLElement("Alias");
  parent.addChild(xalias);
}
 : #(CONST_ALIAS nodeParameter[xalias] value[xalias]) ;
  
varAlias[IXMLElement parent]
{
  IXMLElement xalias = new XMLElement("Alias");
  parent.addChild(xalias);
}
 : #(VARIABLE_ALIAS nodeParameter[xalias] variable[xalias]) ;
  
nodeParameter[IXMLElement parent] : vn:variableName
  {
    IXMLElement xNodeParm = new XMLElement("NodeParameter");
    xNodeParm.setContent(#vn.getText());
    parent.addChild(xNodeParm);
  }
;


//
// Expressions
//

booleanTerm[IXMLElement parent] : 
   booleanVariable[parent]
   | booleanArrayReference[parent]
   | bval:booleanValue { parent.addChild(((PlexilASTNode) #bval).getXmlElement()); }
   | isKnownExp[parent]
   | lookup[parent] ;

booleanValue :
   ( i:INT { #i.getText().equals("0") || #i.getText().equals("1") }?
     | TRUE_KYWD
     | FALSE_KYWD
   )
   {
     ((PlexilASTNode) #booleanValue).setDataType(PlexilDataType.BOOLEAN_TYPE);
   }
 ;

isKnownExp[IXMLElement parent]
{ IXMLElement xisKnown = new XMLElement("IsKnown"); parent.addChild(xisKnown); }
 :
        #(IS_KNOWN_KYWD 
            ( variable[xisKnown]
              | nodeStateVariable[xisKnown]
              | nodeOutcomeVariable[xisKnown]
              | nodeTimepointValue[xisKnown] )
         ) ;

booleanExpression[IXMLElement parent] :
 or[parent]
 | and[parent]
 | not[parent]
 | gt[parent]
 | ge[parent]
 | lt[parent]
 | le[parent]
 | comparison[parent]
 | booleanTerm[parent] ;

or[IXMLElement parent]
{ IXMLElement xor = new XMLElement("OR"); }
 :
   #(OR_KYWD (booleanExpression[xor])+ )
   {
     parent.addChild(xor);
   }
 ;

and[IXMLElement parent]
{ IXMLElement xand = new XMLElement("AND"); }
 : 
   #(AND_KYWD (booleanExpression[xand])+ )
   {
     parent.addChild(xand);
   }
 ;

not[IXMLElement parent]
{ IXMLElement xnot = new XMLElement("NOT"); }
 : 
   #(BANG booleanExpression[xnot])
   {
     parent.addChild(xnot);
   }
 ;

gt[IXMLElement parent]
{ IXMLElement xgt = new XMLElement("GT"); }
 : 
   #(GREATER numericExpression[xgt] numericExpression[xgt])
   {
     parent.addChild(xgt);
   }
 ;

ge[IXMLElement parent]
{ IXMLElement xge = new XMLElement("GE"); }
 : 
   #(GEQ numericExpression[xge] numericExpression[xge])
   {
     parent.addChild(xge);
   }
 ;

lt[IXMLElement parent]
{ IXMLElement xlt = new XMLElement("LT"); }
 : 
   #(LESS numericExpression[xlt] numericExpression[xlt])
   {
     parent.addChild(xlt);
   }
 ;

le[IXMLElement parent]
{ IXMLElement xle = new XMLElement("LE"); }
 : 
   #(LEQ numericExpression[xle] numericExpression[xle])
   {
     parent.addChild(xle);
   }
 ;	

comparison[IXMLElement parent] :
 booleanComparison[parent]
 | numericComparison[parent]
 | nodeStateComparison[parent]
 | nodeOutcomeComparison[parent]
 | nodeCommandHandleComparison[parent]
 | nodeFailureComparison[parent]
 | stringComparison[parent]
 | timeComparison[parent] ;

booleanComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQBoolean"); }
 : 
   #(BOOLEAN_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEBoolean"); } )
     booleanExpression[xeq]
     booleanExpression[xeq])
   {
     parent.addChild(xeq);
   }
 ;

numericComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQNumeric"); }
 : 
   #(NUMERIC_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NENumeric"); } )
     numericExpression[xeq]
     numericExpression[xeq])
   {
     parent.addChild(xeq);
   }
 ;

nodeStateComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQInternal"); }
 : 
   #(NODE_STATE_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEInternal"); } )
     nodeState[xeq]
     nodeState[xeq])
   {
     parent.addChild(xeq);
   }
   ;

nodeOutcomeComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQInternal"); }
 : 
   #(NODE_OUTCOME_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEInternal"); } )
     nodeOutcome[xeq]
     nodeOutcome[xeq])
   {
     parent.addChild(xeq);
   }
   ;

nodeCommandHandleComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQInternal"); }
 : 
   #(NODE_COMMAND_HANDLE_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEInternal"); } )
     nodeCommandHandle[xeq]
     nodeCommandHandle[xeq])
   {
     parent.addChild(xeq);
   }
   ;

nodeFailureComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQInternal"); }
 : 
   #(NODE_FAILURE_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEInternal"); } )
     nodeFailure[xeq]
     nodeFailure[xeq])
   {
     parent.addChild(xeq);
   }
   ;

stringComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQString"); }
 :
   #(STRING_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NEString"); } )
     stringExpression[xeq]
     stringExpression[xeq])
   {
     parent.addChild(xeq);
   }
 ;

timeComparison[IXMLElement parent]
{ IXMLElement xeq = new XMLElement("EQTime"); }
 : 
   #(TIME_COMPARISON 
     (DEQUALS
      | NEQUALS { xeq.setName("NETime"); } )
     timeExpression[xeq]
     timeExpression[xeq])
   {
     parent.addChild(xeq);
   }
 ;

nodeState[IXMLElement parent] :
   nodeStateVariable[parent]
   | nodeStateValue[parent]
 ;

nodeStateVariable[IXMLElement parent]
{ IXMLElement nsv = new XMLElement("NodeStateVariable"); }
 :
   #(NODE_STATE_VARIABLE nodeIdRef[nsv])
   {
     parent.addChild(nsv);
   }
 ;

// nodeNameRef doesn't check whether node name is known

nodeNameRef[IXMLElement parent] :
   n:NodeName
   {
     IXMLElement nid = new XMLElement("NodeRef");
     nid.setContent(n.getText());
     parent.addChild(nid);
   }
;

// nodeIdRef expects node name to be known

nodeIdRef[IXMLElement parent]
{ IXMLElement nid = new XMLElement("NodeId"); }
 :
   n:NodeName { state.isNodeName(n.getText()) }?
   {
     nid.setContent(n.getText());
     parent.addChild(nid);
   }
;

// nodeRef expects node name to be known

nodeRef[IXMLElement parent]
{ IXMLElement nid = new XMLElement("NodeRef"); }
 :
   n:NodeName { state.isNodeName(n.getText()) }?
   {
     nid.setContent(n.getText());
     parent.addChild(nid);
   }
;

// libraryNodeIdRef expects node name to be a known library node

libraryNodeIdRef[IXMLElement parent]
{ IXMLElement nid = new XMLElement("NodeId"); }
 :
   n:NodeName { PlexilGlobalContext.getGlobalContext().isLibraryNodeName(n.getText()) }?
   {
     nid.setContent(n.getText());
     parent.addChild(nid);
   }
;

nodeStateValue[IXMLElement parent]
{ IXMLElement nsv = new XMLElement("NodeStateValue"); }
 :
   ( WAITING_STATE_KYWD
     | EXECUTING_STATE_KYWD
     | FINISHING_STATE_KYWD
     | FAILING_STATE_KYWD
     | FINISHED_STATE_KYWD
     | ITERATION_ENDED_STATE_KYWD
     | INACTIVE_STATE_KYWD
     )
   {
     nsv.setContent(#nodeStateValue.getText());
     parent.addChild(nsv);
   }
 ;

nodeOutcome[IXMLElement parent] : 
   nodeOutcomeVariable[parent]
   | nodeOutcomeValue[parent] ;

nodeOutcomeVariable[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeOutcomeVariable"); }
 :
   #(NODE_OUTCOME_VARIABLE nodeIdRef[nov])
   {
     parent.addChild(nov);
   }
 ;

nodeOutcomeValue[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeOutcomeValue"); }
 :
   ( SUCCESS_OUTCOME_KYWD
     | FAILURE_OUTCOME_KYWD
     | SKIPPED_OUTCOME_KYWD
     | INFINITE_LOOP_OUTCOME_KYWD
   )
   {
     nov.setContent(#nodeOutcomeValue.getText());
     parent.addChild(nov);
   }
 ;

nodeCommandHandle[IXMLElement parent] : 
   nodeCommandHandleVariable[parent]
   | nodeCommandHandleValue[parent] ;

nodeCommandHandleVariable[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeCommandHandleVariable"); }
 :
   #(NODE_COMMAND_HANDLE_VARIABLE nodeIdRef[nov])
   {
     parent.addChild(nov);
   }
 ;

nodeCommandHandleValue[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeCommandHandleValue"); }
 :
   ( COMMAND_SENT_KYWD
     | COMMAND_ACCEPTED_KYWD
     | COMMAND_DENIED_KYWD
     | COMMAND_RCVD_KYWD
     | COMMAND_FAILED_KYWD
     | COMMAND_SUCCESS_KYWD 
   )
   {
     nov.setContent(#nodeCommandHandleValue.getText());
     parent.addChild(nov);
   }
 ;

nodeFailure[IXMLElement parent] : 
   nodeFailureVariable[parent]
   | nodeFailureValue[parent] ;

nodeFailureVariable[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeFailureVariable"); }
 :
   #(NODE_FAILURE_VARIABLE nodeRef[nov])
   {
     parent.addChild(nov);
   }
 ;

nodeFailureValue[IXMLElement parent]
{ IXMLElement nov = new XMLElement("NodeFailureValue"); }
 :
   ( PRE_CONDITION_FAILED_KYWD
     | POST_CONDITION_FAILED_KYWD
     | INVARIANT_CONDITION_FAILED_KYWD
     | PARENT_FAILED_KYWD
     | COMMAND_FAILED_KYWD
     | INFINITE_LOOP_KYWD
   )
   {
     nov.setContent(#nodeFailureValue.getText());
     parent.addChild(nov);
   }
 ;

nodeTimepointValue[IXMLElement parent]
{ IXMLElement ntpv = new XMLElement("NodeTimepointValue"); }
 : #(NODE_TIMEPOINT_VALUE nodeIdRef[ntpv] nodeStateValue[ntpv] timepoint[ntpv])
   {
     parent.addChild(ntpv);
   }
 ;

timepoint[IXMLElement parent]
{ IXMLElement tp = new XMLElement("Timepoint"); }
 :
   ( START_KYWD | END_KYWD )
   {
     tp.setContent(#timepoint.getText());
     parent.addChild(tp);
   }
 ;

// Have to do lookahead here because VARIABLE is ambiguous
// and backtracking is slightly broken

numericTerm[IXMLElement parent] :
// these are the conflicted cases
 { #numericTerm.getType() == VARIABLE && context.isIntegerVariableName(#numericTerm.getText()) }? integerVariable[parent]
 | { #numericTerm.getType() == VARIABLE && context.isRealVariableName(#numericTerm.getText()) }? realVariable[parent]
// end conflicted cases
 | iv:integerValue { parent.addChild(((PlexilASTNode) #iv).getXmlElement()); }
 | rv:realValue { parent.addChild(((PlexilASTNode) #rv).getXmlElement()); }
 | integerArrayReference[parent]
 | realArrayReference[parent]
 | numericUnaryOperation[parent]
 | lookup[parent]
 | nodeTimepointValue[parent]
 | LPAREN! numericExpression[parent] RPAREN! ;

// *** Turn left-associative expressions into right-associative (later)
numericExpression[IXMLElement parent] : 
  numericTerm[parent]
  | add[parent]
  | sub[parent]
  | mul[parent]
  | div[parent] ;

// All numericUnaryOperation ASTs are of the form (numericUnaryOperator numericExpression)
// *** there really ought to be a parametric way to do this!
numericUnaryOperation[IXMLElement parent]
 : absValue[parent]
   | sqrt[parent]
 ;

absValue[IXMLElement parent]
{ IXMLElement xabs = new XMLElement("ABS"); }
 :
   #(ABS_KYWD numericExpression[xabs])
   {
     parent.addChild(xabs);
   }
 ;

sqrt[IXMLElement parent]
{ IXMLElement xsqrt = new XMLElement("SQRT"); }
 :
   #(SQRT_KYWD numericExpression[xsqrt])
   {
     parent.addChild(xsqrt);
   }
 ;

add[IXMLElement parent]
{ IXMLElement xadd = new XMLElement("ADD"); }
 : 
   #(PLUS numericExpression[xadd] (numericExpression[xadd])+ ) 
   {
     parent.addChild(xadd);
   }
   ;

sub[IXMLElement parent]
{ IXMLElement xsub = new XMLElement("SUB"); }
 : 
   #(MINUS numericExpression[xsub] numericExpression[xsub]) 
   {
     parent.addChild(xsub);
   }
   ;

mul[IXMLElement parent]
{ IXMLElement xmul = new XMLElement("MUL"); }
 : 
   #(ASTERISK numericExpression[xmul] (numericExpression[xmul])+ ) 
   {
     parent.addChild(xmul);
   }
   ;

div[IXMLElement parent]
{ IXMLElement xdiv = new XMLElement("DIV"); }
 : 
   #(SLASH numericExpression[xdiv] numericExpression[xdiv]) 
   {
     parent.addChild(xdiv);
   }
   ;

integerValue :
  ( INT )
 { ((PlexilASTNode) #integerValue).setDataType(PlexilDataType.INTEGER_TYPE); }
 ;

realValue :
  ( DOUBLE | INT )
 { ((PlexilASTNode) #realValue).setDataType(PlexilDataType.REAL_TYPE); }
 ;

stringExpression[IXMLElement parent] :
  stringConcatenation[parent]
  | sv:stringValue { parent.addChild(((PlexilASTNode) #sv).getXmlElement()); }
  | stringVariable[parent]
  | stringArrayReference[parent]
  | lookup[parent] ;

stringConcatenation[IXMLElement parent] 
{ IXMLElement xconcat = new XMLElement("Concat"); }
 :
  #(CONCAT stringExpression[xconcat] stringExpression[xconcat])
  {
    parent.addChild(xconcat);
  }
  ;

stringValue : 
   ( STRING )
 { ((PlexilASTNode) #stringValue).setDataType(PlexilDataType.STRING_TYPE); }
 ;

timeExpression[IXMLElement parent] :
 tv:timeValue { parent.addChild(((PlexilASTNode)#tv).getXmlElement()); }
 | timeVariable[parent]
 | lookup[parent] ;


//
// Lookups
//

lookup[IXMLElement parent] :
 lookupWithFrequency[parent]
 | lookupOnChange[parent]
 | lookupNow[parent] ;

lookupWithFrequency[IXMLElement parent]
{ 
  IXMLElement xlwf = new XMLElement("LookupWithFrequency"); 
  parent.addChild(xlwf); 
}
 :
   #(LOOKUP_WITH_FREQ_KYWD stateName[xlwf] frequency[xlwf] (argumentList[xlwf])? )
 ;

frequency[IXMLElement lookup]
{ IXMLElement xfreq = new XMLElement("Frequency"); }
 : 
   #(LOOKUP_FREQUENCY lowFreq[xfreq] highFreq[xfreq])
   {
     lookup.addChild(xfreq);
   }
 ;

lowFreq[IXMLElement parent]
{ IXMLElement lo = new XMLElement("Low"); }
 :
   ( rval:realValue { lo.addChild(((PlexilASTNode) #rval).getXmlElement()); }
     | realVariable[lo] )
   {
     parent.addChild(lo);
   }
 ;

highFreq[IXMLElement parent]
{ IXMLElement hi = new XMLElement("High"); }
 :
   ( rval:realValue { hi.addChild(((PlexilASTNode) #rval).getXmlElement()); }
     | realVariable[hi] )
   {
     parent.addChild(hi);
   }
 ;

lookupOnChange[IXMLElement parent]
{
  IXMLElement xloc = new XMLElement("LookupOnChange");
  parent.addChild(xloc);
}
 :
   #(LOOKUP_ON_CHANGE_KYWD stateName[xloc] (tolerance[xloc])? (argumentList[xloc])? ) ;

tolerance[IXMLElement parent]
{ IXMLElement xtol = new XMLElement("Tolerance"); parent.addChild(xtol); }
 :
 ( rval:realValue { xtol.addChild(((PlexilASTNode) #rval).getXmlElement()); }
   | variable[xtol]
 )
;

lookupNow[IXMLElement parent]
{ 
  IXMLElement xln = new XMLElement("LookupNow"); 
  parent.addChild(xln);
}
 :
   #(LOOKUP_NOW_KYWD stateName[xln] ( argumentList[xln] )? ) ; 

stateName[IXMLElement parent]
{ IXMLElement sn = new XMLElement("Name"); }
 : n:NCName
   {
     IXMLElement sv = new XMLElement("StringValue");
     sv.setContent(n.getText());
     sn.addChild(sv);
     parent.addChild(sn);
   }
 ;
