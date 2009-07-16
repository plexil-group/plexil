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


// *** To do:
//  - Assignment to array element references
//  - String expressions for cmd/fn/lookup names
//  - Arg #/type checking for statically known cmd/fn/lookup names
//  - BLOBs

header 
{
package plexil;

import java.io.FileNotFoundException;
import java.io.File;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.text.NumberFormat;
import java.util.List;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.util.Vector;

}

class PlexilParser extends Parser;
options 
{
  k = 3; // ???
  exportVocab = Plexil;
  buildAST = true;
  defaultErrorHandler = true; // Generate parser error handler
}

tokens 
{
    //
    // Keywords
    //

    COMMENT_KYWD="Comment";

    //
    // Kinds of declarations
    COMMAND_KYWD="Command";
    FUNCTION_KYWD="Function";
    LOOKUP_KYWD="Lookup";
    LIBRARY_NODE_KYWD="LibraryNode";

    // Useful in above decls
    RETURNS_KYWD="returns";

    // resources

    RESOURCE_KYWD="Resource";
    RESOURCE_PRIORITY_KYWD="ResourcePriority";
    NAME_KYWD="Name";
    UPPER_BOUND_KYWD="UpperBound";
    LOWER_BOUND_KYWD="LowerBound";
    RELEASE_AT_TERM_KYWD="ReleaseAtTermination";


    PRIORITY_KYWD="Priority";
    PERMISSIONS_KYWD="Permissions";

    //
    // Conditions
    //
    START_CONDITION_KYWD="StartCondition";
    REPEAT_CONDITION_KYWD="RepeatCondition";
    SKIP_CONDITION_KYWD="SkipCondition";
    PRE_CONDITION_KYWD="PreCondition";
    POST_CONDITION_KYWD="PostCondition";
    INVARIANT_CONDITION_KYWD="InvariantCondition";
    END_CONDITION_KYWD="EndCondition";
    // condition synonyms
    START_COND_KYWD="Start";
    REPEAT_KYWD="Repeat";
    SKIP_KYWD="Skip";
    PRE_KYWD="Pre";
    POST_KYWD="Post";
    INVARIANT_KYWD="Invariant";
    END_COND_KYWD="End";

    IN_KYWD="In";
    IN_OUT_KYWD="InOut";

    BOOLEAN_KYWD="Boolean";
    INTEGER_KYWD="Integer";
    REAL_KYWD="Real";
    STRING_KYWD="String";
    BLOB_KYWD="BLOB";
    TIME_KYWD="Time";

    TRUE_KYWD="true"<AST=plexil.LiteralASTNode>;
    FALSE_KYWD="false"<AST=plexil.LiteralASTNode>;

    // node types
    NODE_LIST_KYWD="NodeList";
    FUNCTION_CALL_KYWD="FunctionCall";
    ASSIGNMENT_KYWD="Assignment";
    UPDATE_KYWD="Update";
    REQUEST_KYWD="Request";
    LIBRARY_CALL_KYWD="LibraryCall";

    OR_KYWD="OR";
    AND_KYWD="AND";
    NOT_KYWD="NOT";

    STATE_KYWD="state";
    OUTCOME_KYWD="outcome";
    COMMAND_HANDLE_KYWD="command_handle";
    FAILURE_KYWD="failure";

    WAITING_STATE_KYWD="WAITING";
    EXECUTING_STATE_KYWD="EXECUTING";
    FINISHING_STATE_KYWD="FINISHING";
    FAILING_STATE_KYWD="FAILING";
    FINISHED_STATE_KYWD="FINISHED";
    ITERATION_ENDED_STATE_KYWD="ITERATION_ENDED";
    INACTIVE_STATE_KYWD="INACTIVE";

    // node outcome values
    SUCCESS_OUTCOME_KYWD="SUCCESS";
    FAILURE_OUTCOME_KYWD="FAILURE";
    SKIPPED_OUTCOME_KYWD="SKIPPED";
    INFINITE_LOOP_KYWD="INFINITE_LOOP";

    // command handle values
    COMMAND_SENT_KYWD="COMMAND_SENT_TO_SYSTEM";
    COMMAND_ACCEPTED_KYWD="COMMAND_ACCEPTED";
    COMMAND_DENIED_KYWD="COMMAND_DENIED";
    COMMAND_RCVD_KYWD="COMMAND_RCVD_BY_SYSTEM";
    COMMAND_FAILED_KYWD="COMMAND_FAILED";
    COMMAND_SUCCESS_KYWD="COMMAND_SUCCESS";

    // failure values
    PRE_CONDITION_FAILED_KYWD="PRE_CONDITION_FAILED";
    POST_CONDITION_FAILED_KYWD="POST_CONDITION_FAILED";
    INVARIANT_CONDITION_FAILED_KYWD="INVARIANT_CONDITION_FAILED";
    PARENT_FAILED_KYWD="PARENT_FAILED";
    // INFINITE_LOOP_KYWD="INFINITE_LOOP"; // see outcome values above

    // these are confusingly similar to condition synonyms
    START_KYWD="START";
    END_KYWD="END";

    LOOKUP_WITH_FREQ_KYWD="LookupWithFrequency";
    LOOKUP_ON_CHANGE_KYWD="LookupOnChange";
    LOOKUP_NOW_KYWD="LookupNow";

    SQRT_KYWD="sqrt";
    ABS_KYWD="abs";
    IS_KNOWN_KYWD="isKnown";

    // lexical token types that need a specific AST node type
    INT<AST=plexil.LiteralASTNode>;
    DOUBLE<AST=plexil.LiteralASTNode>;
    STRING<AST=plexil.StringLiteralASTNode>;

    // artificially constructed tokens
    NCName;
    NodeName;

    // tokens used in the AST
    PLEXIL; // a root node, so that there is ALWAYS an AST
    NODE<AST=plexil.NodeASTNode>;
    NODE_ID;
    GLOBAL_DECLARATIONS;
    PARAMETER_DECLARATIONS;
    PARAMETER;
    RETURN_VALUE;
    VARIABLE<AST=plexil.VariableASTNode>;
    VARIABLE_DECLARATION;

    ALIASES;
    CONST_ALIAS;
    VARIABLE_ALIAS;

    ARRAY_ASSIGNMENT;
    // BLOB_ASSIGNMENT; // not used at present
    BOOLEAN_ASSIGNMENT;
    INTEGER_ASSIGNMENT;
    REAL_ASSIGNMENT;
    STRING_ASSIGNMENT;
    TIME_ASSIGNMENT;

    BLOB_ARRAY;
    BOOLEAN_ARRAY;
    INTEGER_ARRAY;
    REAL_ARRAY;
    STRING_ARRAY;
    TIME_ARRAY;

    ARRAY_REF<AST=plexil.ArrayRefASTNode>;

    BOOLEAN_ARRAY_LITERAL<AST=plexil.LiteralASTNode>;
    INTEGER_ARRAY_LITERAL<AST=plexil.LiteralASTNode>;
    REAL_ARRAY_LITERAL<AST=plexil.LiteralASTNode>;
    STRING_ARRAY_LITERAL<AST=plexil.LiteralASTNode>;
    TIME_ARRAY_LITERAL<AST=plexil.LiteralASTNode>;

    // hmm, don't seem to be used yet
    // BLOB_EXPRESSION;
    // BOOLEAN_EXPRESSION;
    // INTEGER_EXPRESSION;
    // NUMERIC_EXPRESSION;
    // REAL_EXPRESSION;
    // STRING_EXPRESSION;
    // TIME_EXPRESSION;

    BOOLEAN_COMPARISON;
    NODE_STATE_COMPARISON;
    NODE_OUTCOME_COMPARISON;
    NODE_COMMAND_HANDLE_COMPARISON;
    NODE_FAILURE_COMPARISON;
    NUMERIC_COMPARISON;
    STRING_COMPARISON;
    TIME_COMPARISON;

    LOOKUP_FREQUENCY;
    LOOKUP_STATE;
    POINTS_TO;
    TIME_VALUE<AST=plexil.TimeLiteralASTNode>;
    NODE_STATE_VARIABLE;
    NODE_OUTCOME_VARIABLE;
    NODE_COMMAND_HANDLE_VARIABLE;
    NODE_FAILURE_VARIABLE;
    NODE_TIMEPOINT_VALUE;
    ARGUMENT_LIST;
    COMMAND_NAME;
    FUNCTION_NAME;
    STATE_NAME;
    CONCAT;
}

{
    static final int PLEXIL_MAJOR_RELEASE = 1;
    static final int PLEXIL_MINOR_RELEASE = 1;
    static final int PLEXIL_PARSER_RELEASE = 0;

    PlexilParserState state = new PlexilParserState();
    PlexilGlobalContext globalContext = PlexilGlobalContext.getGlobalContext();
    PlexilNodeContext context = globalContext;
    NumberFormat numberFormat = NumberFormat.getInstance();

    // For the convenience of the plan editor ONLY!!

    public void setContext(PlexilNodeContext ctxt)
    {
        context = ctxt;
    }

    public void mergeStates(PlexilParserState toAdd) 
    {
        state.putAll(toAdd);
    }

    public PlexilParserState getState() 
    {
        return state;
    }

    public String toString()
    {
        return "PlexilParser " +
               PLEXIL_MAJOR_RELEASE + "." +
               PLEXIL_MINOR_RELEASE + "." +
               PLEXIL_PARSER_RELEASE;
    }

    //
    // Main fns
    //

    public static PlexilParser parse(File file) 
    throws RecognitionException, TokenStreamException, FileNotFoundException
    {
        return parse(null,file,null);
    }

    public static PlexilParser parse(File file, FileWriter debugWriter)
    throws RecognitionException, TokenStreamException, FileNotFoundException
    {
        return parse(null,file,debugWriter);
    }

    public static PlexilParser parse(PlexilParserState init, File file, FileWriter debugWriter)
    throws RecognitionException, TokenStreamException, FileNotFoundException
    {
        BufferedReader reader = null;
        reader = new BufferedReader(new FileReader(file));
        PlexilLexer lexer = new PlexilLexer(reader);
        PlexilParser parser = new PlexilParser(lexer);
        parser.setASTFactory(new PlexilASTFactory(file));
        if (init!=null)
            parser.mergeStates(init);

        parser.getState().pushFile(file);
        parser.plexilPlan();
        parser.getState().popFile();
        return parser;
    }

  private SemanticException createSemanticException(String message, antlr.collections.AST location)
  {
    PlexilASTNode plexLoc = null;
    if (location != null)
    {
      plexLoc = (PlexilASTNode) location;
    }
    if (plexLoc == null)
    {
      int line = 0;
      int col = 0;
      try
      {
        line = LT(0).getLine();
        col = LT(0).getColumn();
      }
      catch (TokenStreamException e)
      {}
      return new SemanticException(message,
                                   state.getFile().getPath(),
                                   line,
                                   col);
    }
    else
    {
      return new SemanticException(message,
                                   plexLoc.getFilename(),
                                   plexLoc.getLine(),
                                   plexLoc.getColumn());
    }
  }

  private SemanticException createSemanticException(String message)
    throws TokenStreamException
  {
    return createSemanticException(message,null);
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

  /*
    Methods involving state
  */

  public void reportError(RecognitionException ex)
  {
    state.error(ex);
  }
  public void reportWarning(RecognitionException ex)
  {
    state.warn(ex);
  }
}

//
// Plexil grammar begins here
// 

plexilPlan :
        (declarations)?
        (node) EOF!
    { #plexilPlan = #(#[PLEXIL, "PLEXIL"], #plexilPlan) ;
      ((PlexilASTNode) #plexilPlan).setFilename(state.getFile().toString());
    }
    ;

declarations : (declaration)+
        { #declarations = #(#[GLOBAL_DECLARATIONS, "GlobalDeclarations"], #declarations); }
            ;

declaration :
    commandDeclaration
    | functionDeclaration
    | lookupDeclaration
    | libraryNodeDeclaration ;


// should generate #(COMMAND_KYWD commandName (returnsSpec)? (paramsSpec)?)
// return type may be null!

commandDeclaration :
        ( (tn:typeName!)? COMMAND_KYWD^ cn:ncName (p:paramsSpec!)? SEMICOLON! )
        { 
            // add return spec (if needed)
            AST return_spec = null;
            if (#tn != null)
            {
                return_spec = #(#[RETURNS_KYWD, "returns"],
                                #(#[RETURN_VALUE, "RETURN_VALUE"], #tn));
                #commandDeclaration.addChild(return_spec);
            }

            // process params
            // *** add error checking ***
            if (#p != null)
            {
                #commandDeclaration.addChild(#p);
            }

            globalContext.addCommandName(#cn.getText(), #p, return_spec);
        };

// *** To do:
//  - multiple return values

// should generate #(LOOKUP_KYWD stateName (returnsSpec)* (paramsSpec)*)

lookupDeclaration!
 { AST state_name = null;
   AST parm_spec = null;
   AST return_spec = null;
 } : 
        // old style syntax
        ( tn:typeName LOOKUP_KYWD! s:ncName (p:paramsSpec)? SEMICOLON!
        { 
            state_name = #s;
            parm_spec = #p;
            return_spec = #(#[RETURNS_KYWD, "returns"],
                            #(#[RETURN_VALUE, "RETURN_VALUE"], #tn));
        }
        | 
        // multiple return syntax
        LOOKUP_KYWD! sn:stateName (ps:paramsSpec)? RETURNS_KYWD! rs:returnsSpec SEMICOLON!
        { 
            state_name = #sn;
            parm_spec = #ps;
            return_spec = #rs;
        }
        )
        {
            // Actions for either syntax
            #lookupDeclaration = #(#[LOOKUP_KYWD, "Lookup"]);
            #lookupDeclaration.addChild(state_name);
            #lookupDeclaration.addChild(return_spec);
            if (parm_spec != null)
            {
                #lookupDeclaration.addChild(parm_spec);
            }

            // *** to do:
            //  - massage param spec data
            //  - massage return spec data
            // *** may throw exception if lookup already defined!
            globalContext.addLookupName(#s.getText(), parm_spec, return_spec);
        } ;

// should generate #(FUNCTION_KYWD functionName (returnsSpec)? (paramsSpec)?)
// return type may be null!

functionDeclaration : 
        ( (tn:typeName!)? FUNCTION_KYWD^ fn:ncName (p:paramsSpec!)? SEMICOLON! )
        { 
            // add return spec (if supplied)
            AST return_spec = null;
            if (#tn != null)
            {
                return_spec = #(#[RETURNS_KYWD, "returns"],
                                #(#[RETURN_VALUE, "RETURN_VALUE"], #tn));
                #functionDeclaration.addChild(return_spec);
            }

            // error check params
            // *** NYI ***

            // add param spec
            if (#p != null)
            {
                #functionDeclaration.addChild(#p);
            }

            // declare function
            globalContext.addFunctionName(#fn.getText(), #p, return_spec);
        } ;

paramsSpec :
         LPAREN! ( paramSpec ( COMMA! paramSpec )* )? RPAREN!
    { #paramsSpec = #(#[PARAMETER_DECLARATIONS, "PARAMETER_DECLARATIONS"], #paramsSpec) ; } ;

paramSpec :
        typeName ( paramName )? 
    { #paramSpec = #(#[PARAMETER, "PARAMETER"], #paramSpec) ; } ;

returnsSpec :
        returnSpec ( COMMA! returnSpec )*
    { #returnsSpec = #(#[RETURNS_KYWD, "returns"], #returnsSpec); } ;

returnSpec! :
        typeName ( paramName )?
    { #returnSpec = #(#[RETURN_VALUE, "RETURN_VALUE"], #returnSpec); } ;

typeName :
        BLOB_KYWD
        | BOOLEAN_KYWD
        | INTEGER_KYWD
        | REAL_KYWD
        | STRING_KYWD
        | TIME_KYWD ;

paramName : ncName ;

// *** work in progress ***
libraryNodeDeclaration :
        LIBRARY_NODE_KYWD^ n:nodeName (li:libraryInterfaceSpec)? SEMICOLON!
    {
      globalContext.addLibraryNode(#n.getText(), #li);
    }
;

libraryInterfaceSpec :
         LPAREN! ( libraryParamSpec ( COMMA! libraryParamSpec )* )? RPAREN!
    { #libraryInterfaceSpec = #(#[PARAMETER_DECLARATIONS, "PARAMETER_DECLARATIONS"],
                                #libraryInterfaceSpec) ; } ;

libraryParamSpec :
        ( IN_KYWD^ | IN_OUT_KYWD^ ) typeName paramName 
    {
    }
 ;

//
// Nodes
//

node
{ 
  String nodeName = null; 
}
 :
       (myId:nodeId COLON! { nodeName = #myId.getFirstChild().getText(); })?
       lbrace:LBRACE!
       {
         #node = #(#[NODE, "NODE"], #node);
         ((PlexilASTNode) #node).setLine(#lbrace.getLine());
         ((PlexilASTNode) #node).setColumn(#lbrace.getColumn());
         ((PlexilASTNode) #node).setFilename(state.getFile().toString());

         if (#myId == null)
           {
             // create default node name & AST
             nodeName = context.generateChildNodeName();
             Token nn = new antlr.CommonToken(NodeName, nodeName);
             nn.setLine(#lbrace.getLine());
             nn.setColumn(#lbrace.getColumn());
             AST nnAST = astFactory.create(nn);
             #myId = #(#[NODE_ID, "NodeId"]);
             #myId.addChild(nnAST);
             #node.addChild(#myId);
           }
         else
           {
             nodeName = #myId.getFirstChild().getText();
           }

         context = new PlexilNodeContext(context, nodeName);
         ((NodeASTNode) #node).setContext(context);
         ((NodeASTNode) #node).setNodeName(nodeName);
         state.addNode(nodeName, context);
       }
       (comment)?
       (nodeDeclaration)*
       (nodeAttribute)*
       (nodeBody)?
       RBRACE!
       {
         context = context.getParentContext(); // back out to previous
       }
    ;

nodeId : nodeName
        { #nodeId = #(#[NODE_ID, "NodeId"], #nodeId); };

comment : COMMENT_KYWD^ COLON! STRING SEMICOLON! ;

nodeBody : 
   nodeList
   | assignment
   | command
   | functionCall
   | update
   | request
   | libraryCall
 ;

nodeList : NODE_LIST_KYWD^ COLON! (node)* ;

nodeDeclaration :
        interfaceDeclaration
        | variableDeclaration;

nodeAttribute :
        startCondition
        | repeatCondition
        | skipCondition
        | preCondition
        | postCondition
        | invariantCondition
        | endCondition
        | priority
        | resource
        | resourcePriority
        | permissions ;

startCondition : ( START_CONDITION_KYWD! | START_COND_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #startCondition = #(#[START_CONDITION_KYWD, "StartCondition"], #startCondition); } ;

repeatCondition :
        ( REPEAT_CONDITION_KYWD! | REPEAT_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #repeatCondition = #(#[REPEAT_CONDITION_KYWD, "RepeatCondition"], #repeatCondition); } ; 

skipCondition :
        ( SKIP_CONDITION_KYWD! | SKIP_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #skipCondition = #(#[SKIP_CONDITION_KYWD, "SkipCondition"], #skipCondition); } ; 

preCondition : ( PRE_CONDITION_KYWD! | PRE_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #preCondition = #(#[PRE_CONDITION_KYWD, "PreCondition"], #preCondition); } ;	

postCondition : ( POST_CONDITION_KYWD! | POST_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #postCondition = #(#[POST_CONDITION_KYWD, "PostCondition"], #postCondition); } ;	

invariantCondition : ( INVARIANT_CONDITION_KYWD! | INVARIANT_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #invariantCondition = #(#[INVARIANT_CONDITION_KYWD, "InvariantCondition"], #invariantCondition); } ;	

endCondition : ( END_CONDITION_KYWD! | END_COND_KYWD! ) COLON! booleanExpression SEMICOLON!
    { #endCondition = #(#[END_CONDITION_KYWD, "EndCondition"], #endCondition); } ;  


resource
{
  AST lbExpr = null;
  AST ubExpr = null;
  AST releaseExpr = null;
  AST prioExpr = null;
}
 :
    RESOURCE_KYWD^ COLON! NAME_KYWD! EQUALS! stringExpression
        ( COMMA!
          ( ( LOWER_BOUND_KYWD EQUALS! lbe:numericExpression
              { if (lbExpr == null)
                {
                  lbExpr = #lbe;                    
                }
                else
                {
                  throw createSemanticException("Multiple LowerBound specifications in resource",
                                                #lbe);
                }
              } )
            | ( UPPER_BOUND_KYWD EQUALS! ube:numericExpression
              { if (ubExpr == null)
                {
                  ubExpr = #ube;
                }
                else
                {
                  throw createSemanticException("Multiple UpperBound specifications in resource",
                                                #ube);
                }
              } )
            | ( RELEASE_AT_TERM_KYWD EQUALS! re:booleanExpression
              { if (releaseExpr == null)
                {
                  releaseExpr = #re;
                }
                else
                {
                  throw createSemanticException("Multiple ReleaseAtTermination specifications in resource",
                                                #re);
                }
              } )
            | ( PRIORITY_KYWD EQUALS! pe:numericExpression
              { if (prioExpr == null)
                {
                  prioExpr = #pe;
                }
                else
                {
                  throw createSemanticException("Multiple Priority specifications in resource",
                                                #pe);
                }
              } )
          )
        )*
        SEMICOLON!
 {
   context.addResource(#resource);
 }
 ;

resourcePriority :
 RESOURCE_PRIORITY_KYWD^ COLON! numericExpression SEMICOLON!
 {
   context.setResourcePriorityAST(#resourcePriority);
 }
;

priority : PRIORITY_KYWD^ COLON! nonNegativeInteger SEMICOLON! ;

// *** need to add test for non-negative
nonNegativeInteger : INT ;

permissions : PERMISSIONS_KYWD^ COLON! STRING SEMICOLON! ;

interfaceDeclaration : in | inOut ;

// *** need to add type to interface variable declarations??
// The rule is apparently that all variables declared in a node 
// are automatically in/out interface vars for all of that node's descendants.
// So this may only be of use in library node definitions.

in : 
        IN_KYWD^ (tn:typeName)? ncName (COMMA! ncName)* SEMICOLON!
        { 
            AST var = #in.getFirstChild();
            PlexilDataType typ = null;
            if (#tn == null)
              {
                if (context.isLibraryNode())
                  {
                    throw createSemanticException("In statement in library node requires a variable type",
                                                  #in);
                  }
              }
            else
              {
                typ = PlexilDataType.findByName(#tn.getText());
                if (typ == null)
                  {
                    throw createSemanticException("\"" + #tn.getText()
                                                  + "\" is an unknown data type in an In statement",
                                                  #tn);
                  }
                var = var.getNextSibling(); // skip over type
              }
            while (var != null)
              {
                context.declareInterfaceVariable(var.getText(), false, typ);
                var = var.getNextSibling();
              }
        } ;

inOut :
        IN_OUT_KYWD^ (tn:typeName)? ncName (COMMA! ncName)* SEMICOLON!
        {
            AST var = #inOut.getFirstChild();
            PlexilDataType typ = null;
            if (#tn == null)
              {
                if (context.isLibraryNode())
                  {
                    throw createSemanticException("InOut statement in library node missing variable type",
                                                  #inOut);
                  }
              }
            else
              {
                typ = PlexilDataType.findByName(#tn.getText());
                if (typ == null)
                  {
                    throw createSemanticException("\"" + #tn.getText()
                                                  + "\" is an unknown data type in an InOut statement",
                                                  #tn);
                  }
                var = var.getNextSibling(); // skip over type
              }
            while (var != null)
            {
                context.declareInterfaceVariable(var.getText(), true, typ);
                var = var.getNextSibling();
            }
        } ;

// When guessing, the parser doesn't actually build the full AST,
// so we can't depend on the specialized AST being fully populated
// when executing the semantic predicate.
// This could be a problem for NCNames that aren't IDENTs.

booleanVariable : v:variable { context.isBooleanVariableName(#v.getText()) }? ;

integerVariable : v:variable { context.isIntegerVariableName(#v.getText()) }? ;

blobVariable : v:variable { context.isBlobVariableName(#v.getText()) }? ;

realVariable : v:variable { context.isRealVariableName(#v.getText()) }? ;

stringVariable : v:variable { context.isStringVariableName(#v.getText()) }? ;

timeVariable : v:variable { context.isTimeVariableName(#v.getText()) }? ;

arrayVariable : v:variable { context.isArrayVariableName(#v.getText()) }? ;

booleanArrayVariable : v:variable { context.isBooleanArrayVariableName(#v.getText()) }? ;

integerArrayVariable : v:variable { context.isIntegerArrayVariableName(#v.getText()) }? ;

blobArrayVariable : v:variable { context.isBlobArrayVariableName(#v.getText()) }? ;

realArrayVariable : v:variable { context.isRealArrayVariableName(#v.getText()) }? ;

stringArrayVariable : v:variable { context.isStringArrayVariableName(#v.getText()) }? ;

timeArrayVariable : v:variable { context.isTimeArrayVariableName(#v.getText()) }? ;

variable : n:ncName { context.isVariableName(#n.getText()) }?
  { 
    #variable = #[VARIABLE, n_AST.getText()];
    ((VariableASTNode) #variable).setVariable(context.findVariable(#n.getText()));
  }
  ;

// The variable declarations refer to variableName instead of booleanVariable (et al)
// because the variables' types haven't been established yet.

variableDeclaration :
        booleanVariableDeclarations
        | integerVariableDeclarations
        | realVariableDeclarations
        | stringVariableDeclarations
        | blobVariableDeclarations
        | timeVariableDeclarations ;

booleanVariableDeclarations! : 
        { #booleanVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        BOOLEAN_KYWD! (bvd1:booleanVariableDeclaration { #booleanVariableDeclarations.addChild(#bvd1) ; }
                      (COMMA! bvdn:booleanVariableDeclaration { #booleanVariableDeclarations.addChild(#bvdn) ; })* )? SEMICOLON! 
    ;

booleanVariableDeclaration :
        booleanScalarDeclaration | booleanArrayDeclaration ;

booleanScalarDeclaration! : 
    vn:variableName (EQUALS! iv:booleanValue)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.BOOLEAN_TYPE); 
      #booleanScalarDeclaration = #(#[BOOLEAN_KYWD, "Boolean"], #vn);
      if (#iv != null)
        #booleanScalarDeclaration.addChild(#iv) ;
    }
    ;

booleanArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:booleanArrayInitialValue)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.BOOLEAN_TYPE,
                                   #dim.getText());
      #booleanArrayDeclaration = #(#[BOOLEAN_ARRAY, "BOOLEAN_ARRAY"], #vn, #dim);
      if (#iv != null)
        #booleanArrayDeclaration.addChild(#iv) ;
    }
    ;

// Don't bother checking here whether name is unique
variableName : ncName ;

booleanArrayInitialValue : booleanValue | booleanArrayValue ;

booleanArrayValue: HASHPAREN! ( booleanValue )* RPAREN!
 { 
   #booleanArrayValue =
     #(#[BOOLEAN_ARRAY_LITERAL, "BOOLEAN_ARRAY_LITERAL"], #booleanArrayValue);
   ((PlexilASTNode) #booleanArrayValue).setDataType(PlexilDataType.BOOLEAN_ARRAY_TYPE);
 }
 ;

booleanValue: 
  ( { LT(1).getText().equals("0") || LT(1).getText().equals("1") }? INT
    | TRUE_KYWD
    | FALSE_KYWD
  )
  { ((PlexilASTNode) #booleanValue).setDataType(PlexilDataType.BOOLEAN_TYPE); }
 ;

integerVariableDeclarations! :
        { #integerVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        INTEGER_KYWD! (ivd1:integerVariableDeclaration
                       { #integerVariableDeclarations.addChild(#ivd1) ; }
                       (COMMA! ivdn:integerVariableDeclaration
                               { #integerVariableDeclarations.addChild(#ivdn) ; } 
                       )*
                      )? SEMICOLON!;

integerVariableDeclaration :
        integerScalarDeclaration | integerArrayDeclaration ;

integerScalarDeclaration! : 
    vn:variableName (EQUALS! iv:integerValue)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.INTEGER_TYPE); 
      #integerScalarDeclaration = #(#[INTEGER_KYWD, "Integer"], #vn);
      if (#iv != null)
        #integerScalarDeclaration.addChild(#iv) ;
    }
    ;

integerArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:integerArrayInitialValue)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.INTEGER_TYPE,
                                   #dim.getText());
      #integerArrayDeclaration = #(#[INTEGER_ARRAY, "INTEGER_ARRAY"], #vn, #dim);
      if (#iv != null)
        #integerArrayDeclaration.addChild(#iv) ;
    }
    ;

integerArrayInitialValue : integerValue | integerArrayValue ;

integerArrayValue: HASHPAREN! ( integerValue )* RPAREN!
 { 
   #integerArrayValue =
     #(#[INTEGER_ARRAY_LITERAL, "INTEGER_ARRAY_LITERAL"], #integerArrayValue);
   ((PlexilASTNode) #integerArrayValue).setDataType(PlexilDataType.INTEGER_ARRAY_TYPE);
 }
 ;

integerValue : INT
 {
   ((PlexilASTNode) #integerValue).setDataType(PlexilDataType.INTEGER_TYPE);
 }
 ;


realVariableDeclarations! : 
        { #realVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        REAL_KYWD! (rvd1:realVariableDeclaration
                    { #realVariableDeclarations.addChild(#rvd1) ; }
                    (COMMA! rvdn:realVariableDeclaration
                            { #realVariableDeclarations.addChild(#rvdn) ; }
                    )*
                   )? SEMICOLON! 
    ;

realVariableDeclaration :
        realScalarDeclaration | realArrayDeclaration ;

realScalarDeclaration! : 
    vn:variableName (EQUALS! iv:realValue)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.REAL_TYPE); 
      #realScalarDeclaration = #(#[REAL_KYWD, "Real"], #vn);
      if (#iv != null)
        #realScalarDeclaration.addChild(#iv) ;
    }
    ;

realArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:realArrayInitialValue)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.REAL_TYPE,
                                   #dim.getText());
      #realArrayDeclaration = #(#[REAL_ARRAY, "REAL_ARRAY"], #vn, #dim);
      if (#iv != null)
        #realArrayDeclaration.addChild(#iv) ;
    }
    ;

realArrayInitialValue : realValue | realArrayValue ;

realArrayValue: HASHPAREN! ( realValue )* RPAREN!
 { 
   #realArrayValue =
     #(#[REAL_ARRAY_LITERAL, "REAL_ARRAY_LITERAL"], #realArrayValue);
   ((PlexilASTNode) #realArrayValue).setDataType(PlexilDataType.REAL_ARRAY_TYPE);
 }
 ;

realValue : DOUBLE | INT
 {
   ((PlexilASTNode) #realValue).setDataType(PlexilDataType.REAL_TYPE);
 }
 ;

stringVariableDeclarations! : 
        { #stringVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        STRING_KYWD! (svd1:stringVariableDeclaration
                      { #stringVariableDeclarations.addChild(#svd1) ; }
                      (COMMA! svdn:stringVariableDeclaration
                              { #stringVariableDeclarations.addChild(#svdn) ; }
                      )*
                     )? SEMICOLON! 
    ;

stringVariableDeclaration :
        stringScalarDeclaration | stringArrayDeclaration ;

stringScalarDeclaration! : 
    vn:variableName (EQUALS! iv:stringValue)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.STRING_TYPE); 
      #stringScalarDeclaration = #(#[STRING_KYWD, "String"], #vn);
      if (#iv != null)
        #stringScalarDeclaration.addChild(#iv) ;
    }
    ;

stringArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:stringArrayInitialValue)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.STRING_TYPE,
                                   #dim.getText());
      #stringArrayDeclaration = #(#[STRING_ARRAY, "STRING_ARRAY"], #vn, #dim);
      if (#iv != null)
        #stringArrayDeclaration.addChild(#iv) ;
    }
    ;

stringArrayInitialValue: stringValue | stringArrayValue ;

stringArrayValue: HASHPAREN! ( stringValue )* RPAREN!
 { 
   #stringArrayValue =
     #(#[STRING_ARRAY_LITERAL, "STRING_ARRAY_LITERAL"], #stringArrayValue);
   ((PlexilASTNode) #stringArrayValue).setDataType(PlexilDataType.STRING_ARRAY_TYPE);
 }
 ;

stringValue : STRING
 {
  ((PlexilASTNode) #stringValue).setDataType(PlexilDataType.STRING_TYPE);
 }
 ;

blobVariableDeclarations! : 
        { #blobVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        BLOB_KYWD! (pvd1:blobVariableDeclaration { #blobVariableDeclarations.addChild(#pvd1) ; }
                      (COMMA! pvdn:blobVariableDeclaration { #blobVariableDeclarations.addChild(#pvdn) ; })* )? SEMICOLON! 
    ;

blobVariableDeclaration :
        blobScalarDeclaration | blobArrayDeclaration ;

blobScalarDeclaration! : 
    vn:variableName (EQUALS! iv:pointsTo)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.BLOB_TYPE); 
      #blobScalarDeclaration = #(#[BLOB_KYWD, "Blob"], #vn);
      if (#iv != null)
        #blobScalarDeclaration.addChild(#iv) ;
    }
    ;

blobArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:pointsTo)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.BLOB_TYPE,
                                   #dim.getText());
      #blobArrayDeclaration = #(#[BLOB_ARRAY, "BLOB_ARRAY"], #vn, #dim);
      if (#iv != null)
        #blobArrayDeclaration.addChild(#iv) ;
    }
    ;

// esn = External Structure Name
pointsTo! : esn:ncName { #pointsTo = #(#[POINTS_TO, "POINTS_TO"], esn) ; }
            ( LPAREN! iv:initialValue RPAREN! { #pointsTo.addChild(#iv); } )?
    ;

initialValue : INT ;

timeVariableDeclarations! : 
        { #timeVariableDeclarations = #(#[VARIABLE_DECLARATION, "VARIABLE_DECLARATION"]) ; }
        TIME_KYWD! (tvd1:timeVariableDeclaration { #timeVariableDeclarations.addChild(#tvd1) ; }
                    (COMMA! tvdn:timeVariableDeclaration { #timeVariableDeclarations.addChild(#tvdn) ; })* )? SEMICOLON! 
    ;

timeVariableDeclaration :
        timeScalarDeclaration | timeArrayDeclaration ;

timeScalarDeclaration! : 
    vn:variableName (EQUALS! iv:timeValue)?
    {
      context.addVariableName(#vn.getText(), PlexilDataType.TIME_TYPE); 
      #timeScalarDeclaration = #(#[TIME_KYWD, "Time"], #vn);
      if (#iv != null)
        #timeScalarDeclaration.addChild(#iv) ;
    }
    ;

timeArrayDeclaration! : 
    vn:variableName LBRACKET! dim:nonNegativeInteger RBRACKET! (EQUALS! iv:timeArrayInitialValue)?
    {
      context.addArrayVariableName(#vn.getText(),
                                   PlexilDataType.TIME_TYPE,
                                   #dim.getText());
      #timeArrayDeclaration = #(#[TIME_ARRAY, "TIME_ARRAY"], #vn, #dim);
      if (#iv != null)
        #timeArrayDeclaration.addChild(#iv) ;
    }
    ;

timeArrayInitialValue : timeValue | timeArrayValue ;

timeArrayValue: HASHPAREN! ( timeValue )* RPAREN!
 { 
   #timeArrayValue =
     #(#[TIME_ARRAY_LITERAL, "TIME_ARRAY_LITERAL"], #timeArrayValue);
   ((PlexilASTNode) #timeArrayValue).setDataType(PlexilDataType.TIME_ARRAY_TYPE);
 }
 ;

// *** add units (later)
timeValue! :
 ( i:INT
   { #timeValue = #(#[TIME_VALUE, "TimeValue"], i); }
   | LBRACKET! units:integerValue COMMA! subunits:integerValue RBRACKET!
   { #timeValue = #(#[TIME_VALUE, "TimeValue"], units, subunits); }
 )
 {
   ((PlexilASTNode) #timeValue).setDataType(PlexilDataType.TIME_TYPE);
 }
 ;

booleanArrayReference :
   { context.isBooleanArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.BOOLEAN_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not a Boolean array",
                                      #ar);
     }
   }
   ;

integerArrayReference :
   { context.isIntegerArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.INTEGER_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not an Integer array",
                                      #ar);
     }
   }
   ;

blobArrayReference :
   { context.isBlobArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.BLOB_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not a BLOB array",
                                      #ar);
     }
   }
   ;

realArrayReference :
   { context.isRealArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.REAL_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not a Real array",
                                      #ar);
     }
   }
   ;

stringArrayReference :
   { context.isStringArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.STRING_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not a String array",
                                      #ar);
     }
   }
   ;

timeArrayReference :
   { context.isTimeArrayVariableName(LT(1).getText()) }?
   ar:arrayReference
   { 
     if (((ArrayRefASTNode) #ar).getDataType() != PlexilDataType.TIME_TYPE)
     {
        throw createSemanticException("Array variable \""
                                      + ((ArrayRefASTNode) #ar).getVariable().getName()
                                      + "\" is not a Time array",
                                      #ar);
     }
   }
   ;

arrayReference :
   var:variable { context.isArrayVariableName(#var.getText()) }?
                LBRACKET! idx:numericExpression RBRACKET!
   {
     #arrayReference = #(#[ARRAY_REF, "ARRAY_REF"], #arrayReference); 
   }
   ;

// *** To do:
//  - add check for resource priorities (?)
//  - add type check between return value and variable (in tree parser)

command
{
  AST lhs = null;
  AST variable = null;
} 
 : 
   COMMAND_KYWD^ COLON!
   ( ( ncName ( LBRACKET | EQUALS ) ) =>
     ( ( ( ncName LBRACKET ) =>
         ar:arrayReference EQUALS
         {
           lhs = #ar; 
           variable = lhs.getFirstChild();
         }          
        |
         ( ncName EQUALS ) =>
         v:variable EQUALS
         {
           lhs = #v; 
           variable = lhs;
         }
       ) 
       commandInvocation
     )
     |
     commandInvocation
     )
   {
     // *** probably should move this check to the tree parser ***

     if (variable != null
         && !context.isAssignableVariableName(variable.getText()))
       {
         throw createSemanticException("Variable '" + variable.getText()
                                       + "' is declared an In interface variable and may not be assigned",
                                       #v);
       }

     Vector nodeResources = context.getResources();
     if (!nodeResources.isEmpty() 
         && (context.getResourcePriorityAST() == null))
     {
       // Check that a priority is supplied for each resource
       for (int i = 0; i < nodeResources.size(); i++)
       {
         AST resource = (AST) nodeResources.get(i);
         AST child = resource.getFirstChild();
         while (child != null)
         {
           if (child.getType() == PRIORITY_KYWD)
             break;
           child = child.getNextSibling();
         }
         if (child == null)
           throw createSemanticException("No priority supplied for resource", resource);
       }
     }
   }
;

commandInvocation :
   (
    commandName
    |
    ( LPAREN! commandNameExp RPAREN! )
   )
   ( LPAREN! ( argumentList )? RPAREN! )? SEMICOLON!
   ;
   

commandName : ncName 
 {
   #commandName = #(#[COMMAND_NAME, "COMMAND_NAME"], #commandName);
 }
 ;

commandNameExp : stringExpression ;

argumentList : argument (COMMA! argument)*
  {
    #argumentList = #(#[ARGUMENT_LIST, "ARGUMENT_LIST"], #argumentList);
  }
 ;

// *** needs rethinking ***
argument : INT | DOUBLE | STRING | booleanValue | variable | arrayReference ;

// *** see command above
functionCall
{
  AST lhs = null;
  AST variable = null;
} 
 : 
   FUNCTION_CALL_KYWD^ COLON!
   (
    ( ncName LBRACKET ) =>
    ar:arrayReference EQUALS functionNameExp
    {
      lhs = #ar; 
      variable = lhs.getFirstChild();
    }          
    |
    ( ncName EQUALS ) =>
    v:variable EQUALS functionNameExp
    {
      lhs = #v; 
      variable = lhs;
    }          
    |
    functionNameExp
   )
   LPAREN! ( argumentList )? RPAREN! SEMICOLON!
   {
     if (variable != null 
         && !context.isAssignableVariableName(variable.getText()))
       {
         throw new antlr.SemanticException("Variable '" + variable.getText()
                                           + "' is declared an In interface variable and may not be assigned");
       }
   }
   ;

functionInvocation :
   (
    functionName
    |
    ( LPAREN! functionNameExp RPAREN! )
   )
   ( LPAREN! ( argumentList )? RPAREN! )? SEMICOLON!
   ;

functionName : ncName 
 {
   #functionName = #(#[FUNCTION_NAME, "FUNCTION_NAME"], #functionName);
 }
 ;

functionNameExp : stringExpression ;

// general form is:
// Assignment : ( <varName> | <arrayReference> ) = <expression>
assignment!
{
  AST lhs = null;
  AST variable = null;
  PlexilDataType assignType = null;
} 
 : 
    ASSIGNMENT_KYWD! COLON!
    ( 
      ( ncName LBRACKET ) => ar:arrayReference
      { 
        lhs = #ar;
        variable = lhs.getFirstChild();
        assignType = context.findVariable(variable.getText()).getArrayElementType();
      }
      | 
      var:variable
      { 
        lhs = #var; 
        variable = lhs;
        assignType = context.findVariable(variable.getText()).getVariableType();
      }
    )
    EQUALS!
    (
     { assignType.isArray() }?
     ae:arrayExpression
     { #assignment = #(#[ARRAY_ASSIGNMENT, "ARRAY_ASSIGNMENT"], lhs, #ae) ; }
     |
     { assignType == PlexilDataType.BOOLEAN_TYPE }?
     be:booleanExpression
     { #assignment = #(#[BOOLEAN_ASSIGNMENT, "BOOLEAN_ASSIGNMENT"], lhs, #be) ; }
     |
     { assignType == PlexilDataType.INTEGER_TYPE }?
     ie:numericExpression
     { #assignment = #(#[INTEGER_ASSIGNMENT, "INTEGER_ASSIGNMENT"], lhs, #ie) ; }
     |
     { assignType == PlexilDataType.REAL_TYPE }?
     re:numericExpression
     { #assignment = #(#[REAL_ASSIGNMENT, "REAL_ASSIGNMENT"], lhs, #re) ; }
     |
     { assignType == PlexilDataType.STRING_TYPE }?
     se:stringExpression
     { #assignment = #(#[STRING_ASSIGNMENT, "STRING_ASSIGNMENT"], lhs, #se) ; }
     |
     { assignType == PlexilDataType.TIME_TYPE }?
     te:timeExpression
     { #assignment = #(#[TIME_ASSIGNMENT, "TIME_ASSIGNMENT"], lhs, #te) ; }
    )
    SEMICOLON!
    {
      if (!context.isAssignableVariableName(variable.getText()))
        {
          throw new antlr.SemanticException("Variable '" + variable.getText()
                                            + "' is declared an In interface variable and may not be assigned");
        }
    }
    ;

//
// Update nodes
//

update : UPDATE_KYWD^ COLON! ( pair ( COMMA! pair )* )? SEMICOLON! ;

//
// Request nodes
//
// Note that nodeName need not be known
//

request : REQUEST_KYWD^ COLON! nodeName ( pair ( COMMA! pair )* )? SEMICOLON! ;

// common to both update and request nodes

pair : ncName EQUALS! ( value | variable | arrayReference | lookupNow ) ;

value :
   integerValue
   | realValue
   | booleanValue
   | stringValue
   | timeValue ;

//
// Library Call nodes
//

libraryCall
{ PlexilGlobalDeclaration lndec = null; }
 :
  LIBRARY_CALL_KYWD^ COLON!
   ln:libraryNodeIdRef
   { 
     if (!globalContext.isLibraryNodeName(#ln.getText()))
       throw createSemanticException("Node name \"" + #ln.getText()
                                      + "\" is not a declared library node",
                                      #ln);
     lndec = globalContext.getLibraryNode(#ln.getText());
   }
   ( aliasSpecs[lndec] )? SEMICOLON! ;

// error checking done at libraryCall above
libraryNodeIdRef: n:nodeName ;

aliasSpecs[PlexilGlobalDeclaration decl] :
  LPAREN! ( aliasSpec[decl] ( COMMA! aliasSpec[decl] )* )? RPAREN!
 { #aliasSpecs = #(#[ALIASES, "ALIASES"], #aliasSpecs); } ;

// must disambiguate
aliasSpec[PlexilGlobalDeclaration decl] : 
  ( (ncName EQUALS ncName) )=> varAlias[decl]
  | constAlias[decl]
 ;

constAlias[PlexilGlobalDeclaration decl] : nodeParameterName[decl] EQUALS! value
 { #constAlias = #([CONST_ALIAS, "CONST_ALIAS"], #constAlias); } ;
varAlias[PlexilGlobalDeclaration decl] : nodeParameterName[decl] EQUALS! variable
 { #varAlias = #([VARIABLE_ALIAS, "VARIABLE_ALIAS"], #varAlias); } ;

nodeParameterName[PlexilGlobalDeclaration decl] :
   v:variableName
 { 
   if (!decl.hasParameterNamed(#v.getText()))
     throw createSemanticException("\"" + #v.getText()
                                   + "\" is not a valid node parameter name for library node "
                                   + decl.getName(),
                                   #v) ;
 }
 ;


//
// Boolean expressions
//
// Implement operator precedence as in C(++)/Java
//

// logical OR
booleanExpression! :
   t1:booleanTerm
   { #booleanExpression = #t1; }
   ( ( OR_KYWD | OR_ALT_KYWD ) tn:booleanTerm
    { #booleanExpression = #(#[OR_KYWD, "OR"], #booleanExpression, #tn); }
   )*
 ;

// logical AND
booleanTerm! : 
   f1:booleanFactor
   { #booleanTerm = #f1; }
   ( ( AND_KYWD | AND_ALT_KYWD ) fn:booleanFactor
    { #booleanTerm = #(#[AND_KYWD, "AND"], #booleanTerm, #fn); }
   )*
 ;

// *** difficulty here is that without type information,
// a lookup matches numeric, string, and boolean expressions
booleanFactor :
   !(booleanKernel (equalityOperator booleanKernel)? ) =>
   b1:booleanKernel { #booleanFactor = #b1; }
   (bop:equalityOperator b2:booleanKernel
    { #booleanFactor = #(#[BOOLEAN_COMPARISON, "BOOLEAN_COMPARISON"], #bop, #booleanFactor, #b2); })?
   |
   !(numericExpression equalityOperator numericExpression) =>
   ( n1:numericExpression nop:equalityOperator n2:numericExpression
      { #booleanFactor = #(#[NUMERIC_COMPARISON, "NUMERIC_COMPARISON"], #nop, #n1, #n2); } )
   |
   !(stringExpression equalityOperator stringExpression) =>
   ( s1:stringExpression sop:equalityOperator s2:stringExpression
      { #booleanFactor = #(#[STRING_COMPARISON, "STRING_COMPARISON"], #sop, #s1, #s2); } )
   |
   !(timeExpression equalityOperator timeExpression) =>
   ( t1:timeExpression top:equalityOperator t2:timeExpression
      { #booleanFactor = #(#[TIME_COMPARISON, "TIME_COMPARISON"], #top, #t1, #t2); } )
   |
   !( ns1:nodeState nsop:equalityOperator ns2:nodeState
      { #booleanFactor = #(#[NODE_STATE_COMPARISON, "NODE_STATE_COMPARISON"], #nsop, #ns1, #ns2); } )
   |
   !( no1:nodeOutcome noop:equalityOperator no2:nodeOutcome
      { #booleanFactor = #(#[NODE_OUTCOME_COMPARISON, "NODE_OUTCOME_COMPARISON"], #noop, #no1, #no2); } )
   |
   !( nch1:nodeCommandHandle nchop:equalityOperator nch2:nodeCommandHandle
      { #booleanFactor = #(#[NODE_COMMAND_HANDLE_COMPARISON, "NODE_COMMAND_HANDLE_COMPARISON"],
                           #nchop, #nch1, #nch2); } )
   |
   !( nf1:nodeFailure nfop:equalityOperator nf2:nodeFailure
      { #booleanFactor = #(#[NODE_FAILURE_COMPARISON, "NODE_FAILURE_COMPARISON"],
                           #nfop, #nf1, #nf2); } )
 ;

equalityOperator : DEQUALS | NEQUALS ;

booleanKernel :
   !(numericExpression comparisonOperator numericExpression) =>
   e1:numericExpression op:comparisonOperator e2:numericExpression
   { #booleanKernel = #(#op, #e1, #e2); }
   | not
   | simpleBoolean ;

not : BANG^ simpleBoolean ;

simpleBoolean :
   LPAREN! booleanExpression RPAREN!
   | booleanValue
   | booleanArrayReference
   | booleanVariable
   | isKnownExp
   | lookup ;

isKnownExp :
   IS_KNOWN_KYWD^ 
   LPAREN!
   ( 
     ( ncName LBRACKET ) => arrayReference
     | variable
     | nodeStateVariable
     | nodeOutcomeVariable
     | nodeTimepointValue
   )
   RPAREN! ;

// numericComparison! : 
//    e1:numericExpression op:comparisonOperator e2:numericExpression
//    { #numericComparison = #(#op, #e1, #e2); }
//  ;

comparisonOperator : GREATER | GEQ | LESS | LEQ ;

nodeState : nodeStateVariable | nodeStateValue ;

nodeStateVariable! : 
   nid:nodeIdRef PERIOD! STATE_KYWD
   { #nodeStateVariable = #(#[NODE_STATE_VARIABLE, "NODE_STATE_VARIABLE"], nid); }
 ;

nodeStateValue : 
   WAITING_STATE_KYWD
   | EXECUTING_STATE_KYWD
   | FINISHING_STATE_KYWD
   | FAILING_STATE_KYWD
   | FINISHED_STATE_KYWD
   | ITERATION_ENDED_STATE_KYWD
   | INACTIVE_STATE_KYWD ;

nodeOutcome : nodeOutcomeVariable | nodeOutcomeValue ;

nodeOutcomeVariable! : 
   nid:nodeIdRef PERIOD! OUTCOME_KYWD!
   { #nodeOutcomeVariable = #(#[NODE_OUTCOME_VARIABLE, "NODE_OUTCOME_VARIABLE"], nid); }
   ;

nodeOutcomeValue :
    SUCCESS_OUTCOME_KYWD
    | FAILURE_OUTCOME_KYWD
    | SKIPPED_OUTCOME_KYWD
    | INFINITE_LOOP_KYWD ;

nodeCommandHandle : nodeCommandHandleVariable | nodeCommandHandleValue ;

nodeCommandHandleVariable! :
   nid:nodeIdRef PERIOD! COMMAND_HANDLE_KYWD!
   { #nodeCommandHandleVariable = #(#[NODE_COMMAND_HANDLE_VARIABLE, "NODE_COMMAND_HANDLE_VARIABLE"],
                                    nid); }
   ;

nodeCommandHandleValue :
   COMMAND_SENT_KYWD
   | COMMAND_ACCEPTED_KYWD
   | COMMAND_DENIED_KYWD
   | COMMAND_RCVD_KYWD
   | COMMAND_FAILED_KYWD
   | COMMAND_SUCCESS_KYWD ;

nodeFailure : nodeFailureVariable | nodeFailureValue ;

nodeFailureVariable! :
   nid:nodeIdRef PERIOD! FAILURE_KYWD!
   { #nodeFailureVariable = #(#[NODE_FAILURE_VARIABLE, "NODE_FAILURE_VARIABLE"],
                              nid); }
   ;

nodeFailureValue :
   PRE_CONDITION_FAILED_KYWD
   | POST_CONDITION_FAILED_KYWD
   | INVARIANT_CONDITION_FAILED_KYWD
   | PARENT_FAILED_KYWD
   | COMMAND_FAILED_KYWD
   | INFINITE_LOOP_KYWD ;

nodeTimepointValue :
   nodeIdRef PERIOD! nodeStateValue PERIOD! timepoint
   { #nodeTimepointValue = #(#[NODE_TIMEPOINT_VALUE, "NODE_TIMEPOINT_VALUE"], #nodeTimepointValue); }
 ;

// Note that we can't check forward references to nodes defined later
nodeIdRef: nodeName;

timepoint : START_KYWD | END_KYWD ;

//
// Numeric expressions
//
// Make expressions left-associative by manually building prefix AST
//

// Add/subtract type ops
numericExpression! :
    t1:numericTerm
    { #numericExpression = #t1; }
    (on:additiveOperator tn:numericTerm
     { #numericExpression = #(#on, #numericExpression, #tn); }
    )*
 ;

// Same thing for multiply/divide, at a higher precedence
numericTerm! :
    f1:numericFactor
    { #numericTerm = #f1; }
    (on:multiplicativeOperator fn:numericFactor
     { #numericTerm = #(#on, #numericTerm, #fn); }
    )*
 ;

numericFactor :
   LPAREN! numericExpression RPAREN!
   | absValue
   | numericUnaryOperation
   | INT
   | DOUBLE
   | { context.isIntegerArrayVariableName(LT(1).getText()) }? integerArrayReference
   | { context.isRealArrayVariableName(LT(1).getText()) }? realArrayReference
   | { context.isIntegerVariableName(LT(1).getText()) }? integerVariable
   | { context.isRealVariableName(LT(1).getText()) }? realVariable
   | { !context.isVariableName(LT(1).getText()) }? nodeTimepointValue // s/b isNodeName() ?
   | lookup
 ;

additiveOperator : PLUS | MINUS ;

multiplicativeOperator : ASTERISK | SLASH ;

absValue! :
   BAR! e:numericExpression BAR!
   { #absValue = #(#[ABS_KYWD, "abs"], #e); } ;

numericUnaryOperation! :
   op:numericUnaryOperator LPAREN! exp:numericExpression RPAREN!
   { #numericUnaryOperation = #(#op, #exp); }
 ;

// can add more later
numericUnaryOperator :
   SQRT_KYWD
   | ABS_KYWD ;

//
// string expressions
//

stringExpression! :
   q1:stringQuantity
   { #stringExpression = #q1; }
   ( PLUS! qn:stringQuantity 
     { #stringExpression = #(#[CONCAT, "CONCAT"], #stringExpression, #qn); }
   )*
   ;

stringQuantity : 
   stringValue | stringArrayReference | stringVariable | lookup ;

//
// Time expressions
//

timeExpression : timeValue | timeArrayReference | timeVariable | lookup ;

//
// Array expressions
// *** needs work ***
//

arrayExpression : lookup ;



//
// Lookups
//

lookup : lookupWithFrequency | lookupOnChange | lookupNow ;

// should produce an AST of the form
// #(LOOKUP_WITH_FREQ_KYWD frequency lookupInvocation)

lookupWithFrequency :
        lwf:LOOKUP_WITH_FREQ_KYWD^ LPAREN! f:frequency COMMA! li:lookupInvocation RPAREN!
 ;

frequency! : LBRACKET! lf:lowFreq COMMA! hf:highFreq RBRACKET!
    { #frequency = #(#[LOOKUP_FREQUENCY, "LOOKUP_FREQUENCY"], lf, hf) ; }
    ;

lowFreq : realValue | realVariable ;

highFreq : realValue | realVariable ;

// should produce an AST of the form
// #(LOOKUP_ON_CHANGE_KYWD lookupInvocation (tolerance)? )
// N.b. tolerance is optional

lookupOnChange :
       LOOKUP_ON_CHANGE_KYWD^ LPAREN! lookupInvocation (COMMA! tolerance)? RPAREN!
;

// *** To do:
//  - check tolerance variable type

tolerance : realValue | variable ;

// should produce an AST of the form
// #(LOOKUP_NOW_KYWD stateNameExp (argumentList)? )

lookupNow :
        LOOKUP_NOW_KYWD^ LPAREN! lookupInvocation RPAREN! ;

lookupInvocation :
  ( stateName
    |
    ( LPAREN! stateNameExp RPAREN! )
  )
  ( LPAREN! (argumentList)? RPAREN! )?
 ;

stateName : ncName 
 {
   #stateName = #(#[STATE_NAME, "STATE_NAME"], #stateName);
 }
 ;

stateNameExp : stringExpression ;

// Here's what to do when we really need an NCName, and we have an IDENT

// *** NEW TOKEN CONTAINS ONLY TEXT OF FIRST IDENT -- FIX!!! ***

ncName :
   ( IDENT
     ( PERIOD       // was ( PERIOD | MINUS )
       (IDENT)? )* )
   { 
     String myName = #ncName.getText();
     for (AST part = #ncName.getNextSibling(); part != null; part = part.getNextSibling())
      {
        myName = myName + part.getText();
      }
     Token nm = new antlr.CommonToken(NCName, myName);
     nm.setLine(#ncName.getLine());
     nm.setColumn(#ncName.getColumn());
     #ncName = astFactory.create(nm) ; 
   }
 ;


// A restricted variant of NCName
// TO DO: permit either NCName or integer

nodeName :
   ( IDENT
     ( ( ( PERIOD !(nodeStateValue | STATE_KYWD | OUTCOME_KYWD) ) => PERIOD
       | MINUS )
       (IDENT)? )* )
   { 
     String myName = #nodeName.getText();
     for (AST part = #nodeName.getNextSibling(); part != null; part = part.getNextSibling())
      {
        myName = myName + part.getText();
      }
     Token nm = new antlr.CommonToken(NodeName, myName);
     nm.setLine(#nodeName.getLine());
     nm.setColumn(#nodeName.getColumn());
     #nodeName = astFactory.create(nm) ; 
   }
 ;


// End Grammar

// Begin Lexer / tokens
class PlexilLexer extends Lexer;
options {
  k = 2;
  exportVocab = Plexil;
  charVocabulary = '\3'..'\177';
}

{
  static final int PLEXIL_MAJOR_RELEASE = 1;
  static final int PLEXIL_MINOR_RELEASE = 0;
  static final int PLEXIL_LEXER_RELEASE = 0;
  public String toString()
  {
    return "PlexilLexer "+
           PLEXIL_MAJOR_RELEASE+"."+
           PLEXIL_MINOR_RELEASE+"."+
           PLEXIL_LEXER_RELEASE;
  }
}

LBRACKET: '[';
RBRACKET: ']';
LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')';
BAR: '|';
OR_ALT_KYWD: "||";
AND_ALT_KYWD: "&&";
LESS: '<' ;
GREATER: '>' ;
LEQ: "<=" ;
GEQ: ">=" ;
COLON: ':';
DCOLON: "::";
DEQUALS: "==";
NEQUALS: "!=";
EQUALS: '=';
ASTERISK: '*';
SLASH: '/';
PERIOD: '.';
HASHPAREN: "#(";

SEMICOLON: ';';
COMMA: ',';
BANG: '!';

STRING: '"' (ESC|~('"'|'\\'))* '"'
      | '\'' (ESC|~('\''|'\\'))* '\''
      ;
ESC: '\\' ('n' | 't' | 'b' | 'f' | '"' | '\'' | '\\' | UNICODE_ESC | NUMERIC_ESC);
protected UNICODE_ESC: 
  'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT;

protected NUMERIC_ESC: 
  QUAD_DIGIT (OCTAL_DIGIT (OCTAL_DIGIT)? )?
  | OCTAL_DIGIT (OCTAL_DIGIT)? ;

protected QUAD_DIGIT: ('0'..'3');
protected OCTAL_DIGIT: ('0'..'7');
protected DIGIT: ('0'..'9');
protected HEX_DIGIT: (DIGIT|'A'..'F'|'a'..'f');
protected PLUS: '+';
protected MINUS: '-';

// a few protected methods to assist in matching floating point numbers
protected EXPONENT:  ('e'|'E') (PLUS | MINUS)? (DIGIT)+;
protected FLOAT_SUFFIX:  'f'|'F'|'d'|'D';

// the following returns tokens: PLUS, MINUS, DOUBLE, INT

// *** Chuck's notes:
// prefixes - 
//  +/- is always decimal, can be either int or double
//  0x is always hexadecimal (must not be signed, no decimal point)
//  0o is always octal (must not be signed, no decimal point)
//  0b is always binary (must not be signed, no decimal point)
//  . is always double (decimal)

INT { int base = 0; } :
   ( '0' !DIGIT ) =>
   ( '0' { base = 10; } // special case for just '0'
     ( ( ('x'|'X' { base = 16; } ) // hex
       (HEX_DIGIT)+
       )
     |
       ( ('o'|'O' { base = 8; } ) // octal
         (OCTAL_DIGIT)+
         )
     |
       ( ('b'|'B' { base = 2; } ) // binary
         ('0'|'1')+   
         )
     |
       ( PERIOD { $setType(DOUBLE); }
         (DIGIT)* 
         (EXPONENT)?
         )
     |
       ( EXPONENT { $setType(DOUBLE); } )
     )?
   )
   | 
   ( PERIOD (DIGIT)+ (EXPONENT)? 
     { $setType(DOUBLE); base = 10; }
   )
   |
   ( (DIGIT)+ 
     (PERIOD (DIGIT)* { $setType(DOUBLE); } )?
     (EXPONENT { $setType(DOUBLE); } )?
     { base = 10; }
   )
   |
   ( ('+' { $setType(PLUS); }
      |
      '-' { $setType(MINUS); })
     ( ( (DIGIT)+ { $setType(INT); }
          (PERIOD (DIGIT)* { $setType(DOUBLE); } )?
          (EXPONENT { $setType(DOUBLE); } )? 
         |
         (PERIOD (DIGIT)+ (EXPONENT)? { $setType(DOUBLE); } )
       )
       { base = 10; }
     )?
   )
 ;

// identifiers may not start with a digit
// Note that NCName is a superset of IDENT

// IDENT: ('a'..'z'|'A'..'Z'|'_')
//        ('a'..'z'|'A'..'Z'|'_'|'0'..'9'
//         | '.' { $setType(NCName); } | '-' { $setType(NCName); } )* ;

IDENT: ('a'..'z'|'A'..'Z'|'_')
       ('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;

// Whitespace -- ignored
WS  
  options {
    paraphrase = "whitespace, such as a space, tab, or newline";
  }
  :  (  ' '
    |  '\t'
    |  '\f'
    // handle newlines
    |  (  "\r\n"  // DOS and earlier
      |  '\r'    // Macintosh
      |  '\n'    // Unix
      )
      { newline(); }
    )
    { _ttype = Token.SKIP; }
  ;

// Single-line comments
SL_COMMENT
  options {
    paraphrase = "a single line comment, delimited by //";
  }
  :  "//"
    (~('\n'|'\r'))* ('\n'|'\r'('\n')?)
    {$setType(Token.SKIP); newline();}
  ;

// multiple-line comments
ML_COMMENT
  options {
    paraphrase = "a multiple line comment, such as /* <comment> */";
  }
  : "/*"
    (options {generateAmbigWarnings=false;}:
      { LA(2)!='/' }? '*'
    | "\r\n"    {newline();}
    | '\r'      {newline();}
    | '\n'      {newline();}
    |  ~('*'|'\n'|'\r')
    )*
    "*/"
    {$setType(Token.SKIP);}
  ;
