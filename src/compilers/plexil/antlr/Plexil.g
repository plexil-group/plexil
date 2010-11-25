// Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

grammar Plexil;

options
{ 
    output = AST;
    language = Java; 
    ASTLabelType = PlexilTreeNode;
//    TokenLabelType = PlexilToken;
    backtrack = false; /* for now */
    memoize = false;   /* for now */
	k = 2; /* for now */
}

tokens
{

//
// Keywords
//

COMMENT_KYWD = 'Comment';

// Kinds of declarations
COMMAND_KYWD = 'Command';
LOOKUP_KYWD =  'Lookup';

RETURNS_KYWD = 'Returns';

// Resources
RESOURCE_KYWD = 'Resource';
RESOURCE_PRIORITY_KYWD = 'ResourcePriority';
NAME_KYWD = 'Name';
UPPER_BOUND_KYWD = 'UpperBound';
LOWER_BOUND_KYWD = 'LowerBound';
RELEASE_AT_TERM_KYWD = 'ReleaseAtTermination';

PRIORITY_KYWD = 'Priority';
PERMISSIONS_KYWD = 'Permissions';

// Interface declarations
IN_KYWD = 'In';
IN_OUT_KYWD = 'InOut';

// Types
BOOLEAN_KYWD = 'Boolean';
INTEGER_KYWD = 'Integer';
REAL_KYWD = 'Real';
STRING_KYWD = 'String';
BLOB_KYWD = 'BLOB';
TIME_KYWD = 'Time';

// node types
UPDATE_KYWD = 'Update';
REQUEST_KYWD = 'Request';
LIBRARY_CALL_KYWD = 'LibraryCall';
LIBRARY_ACTION_KYWD = 'LibraryAction';

STATE_KYWD = 'state';
OUTCOME_KYWD = 'outcome';
COMMAND_HANDLE_KYWD = 'command_handle';
FAILURE_KYWD = 'failure';

WAITING_STATE_KYWD = 'WAITING';
EXECUTING_STATE_KYWD = 'EXECUTING';
FINISHING_STATE_KYWD = 'FINISHING';
FAILING_STATE_KYWD = 'FAILING';
FINISHED_STATE_KYWD = 'FINISHED';
ITERATION_ENDED_STATE_KYWD = 'ITERATION_ENDED';
INACTIVE_STATE_KYWD = 'INACTIVE';

// node outcome values
SUCCESS_OUTCOME_KYWD = 'SUCCESS';
FAILURE_OUTCOME_KYWD = 'FAILURE';
SKIPPED_OUTCOME_KYWD = 'SKIPPED';

// command handle values
COMMAND_ABORTED_KYWD      = 'COMMAND_ABORTED';
COMMAND_ABORT_FAILED_KYWD = 'COMMAND_ABORT_FAILED';
COMMAND_ACCEPTED_KYWD     = 'COMMAND_ACCEPTED';
COMMAND_DENIED_KYWD       = 'COMMAND_DENIED';
COMMAND_FAILED_KYWD       = 'COMMAND_FAILED';
COMMAND_RCVD_KYWD         = 'COMMAND_RCVD_BY_SYSTEM';
COMMAND_SENT_KYWD         = 'COMMAND_SENT_TO_SYSTEM';
COMMAND_SUCCESS_KYWD      = 'COMMAND_SUCCESS';

// failure values
PRE_CONDITION_FAILED_KYWD = 'PRE_CONDITION_FAILED';
POST_CONDITION_FAILED_KYWD = 'POST_CONDITION_FAILED';
INVARIANT_CONDITION_FAILED_KYWD = 'INVARIANT_CONDITION_FAILED';
PARENT_FAILED_KYWD = 'PARENT_FAILED';

// Boolean values
TRUE_KYWD = 'true';
FALSE_KYWD = 'false';

// these are confusingly similar to condition synonyms
START_KYWD = 'START';
END_KYWD = 'END';

// *** DEPRECATED ***
LOOKUP_ON_CHANGE_KYWD = 'LookupOnChange';
LOOKUP_NOW_KYWD = 'LookupNow';

// Boolean expressions
XOR_KYWD = 'XOR';

// numerical expressions
ABS_KYWD = 'abs';
IS_KNOWN_KYWD = 'isKnown';
SQRT_KYWD = 'sqrt';
MOD_KYWD = 'mod';

// Node state predicates (Extended Plexil)
NODE_EXECUTING_KYWD = 'NodeExecuting';
NODE_FAILED_KYWD = 'NodeFailed';
NODE_FINISHED_KYWD = 'NodeFinished';
NODE_INACTIVE_KYWD = 'NodeInactive';
NODE_INVARIANT_FAILED_KYWD = 'NodeInvariantFailed';
NODE_ITERATION_ENDED_KYWD = 'NodeIterationEnded';
NODE_ITERATION_FAILED_KYWD = 'NodeIterationFailed';
NODE_ITERATION_SUCCEEDED_KYWD = 'NodeIterationSucceeded';
NODE_PARENT_FAILED_KYWD = 'NodeParentFailed';
NODE_POSTCONDITION_FAILED_KYWD = 'NodePostconditionFailed';
NODE_PRECONDITION_FAILED_KYWD = 'NodePreconditionFailed';
NODE_SKIPPED_KYWD = 'NodeSkipped';
NODE_SUCCEEDED_KYWD = 'NodeSucceeded';
NODE_WAITING_KYWD = 'NodeWaiting';

// Extended Plexil keywords
CONCURRENCE_KYWD = 'Concurrence';
ON_COMMAND_KYWD = 'OnCommand';
ON_MESSAGE_KYWD = 'OnMessage';
TRY_KYWD = 'Try';
UNCHECKED_SEQUENCE_KYWD = 'UncheckedSequence';

ELSE_KYWD = 'else';
ELSEIF_KYWD = 'elseif';
ENDIF_KYWD = 'endif';
FOR_KYWD = 'for';
IF_KYWD = 'if';
WHILE_KYWD = 'while';

// Extended Plexil message passing
MESSAGE_RECEIVED_KYWD = 'MessageReceived';

LBRACKET = '[';
RBRACKET = ']';
LBRACE = '{';
RBRACE = '}';
LPAREN = '(';
RPAREN = ')';
BAR = '|';
LESS = '<' ;
GREATER = '>' ;
LEQ = '<=' ;
GEQ = '>=' ;
COLON = ':';
DEQUALS = '==';
NEQUALS = '!=';
EQUALS = '=';
ASTERISK = '*';
SLASH = '/';
PERCENT = '%';
HASHPAREN = '#(';

SEMICOLON = ';';
COMMA = ',';

// Numeric types (see lexer grammar)
INT;
DOUBLE;

//
// "Imaginary" tokens
//

PLEXIL; // top node of parse tree

ACTION;
ALIASES;
ARGUMENT_LIST;
ARRAY_LITERAL;
ARRAY_REF;
ARRAY_VARIABLE_DECLARATION;
ASSIGNMENT;
BLOCK;
BOOLEAN_COMPARISON;
COMMAND;
CONCAT;
CONST_ALIAS;
GLOBAL_DECLARATIONS;
NODE_ID;
NODE_TIMEPOINT_VALUE;
PARAMETERS;
SEQUENCE; // used in tree parser
STATE_NAME;
STRING_COMPARISON;
VARIABLE_ALIAS;
VARIABLE_DECLARATION;
VARIABLE_DECLARATIONS;

}

//
// 'scope' decls go here
//

@header
{
package plexil;

import plexil.*;
}

@lexer::header 
{ 
package plexil;
}

@members
{
    GlobalContext m_globalContext = new GlobalContext();
    NodeContext m_context = m_globalContext;
}

///////////////////////////
// *** BEGIN GRAMMAR *** //
///////////////////////////

plexilPlan :
    declarations? action EOF
    -> ^(PLEXIL<PlexilPlanNode> declarations? action)
 ;


declarations : 
    declaration+
    -> ^(GLOBAL_DECLARATIONS<GlobalDeclarationsNode> declaration+) ;

declaration :
    commandDeclaration
  | lookupDeclaration
  | libraryActionDeclaration
 ;

// should generate ^(COMMAND_KYWD NCNAME (returnsSpec)? (paramsSpec)?)
// return type may be null!
// *** TODO: add resource lists ***

commandDeclaration :
    ( 
      // no-return-value variant
      ( COMMAND_KYWD NCNAME paramsSpec? SEMICOLON
        -> ^(COMMAND_KYWD<CommandDeclarationNode> NCNAME paramsSpec?)
	  )
    |
      // return value variant
      ( rt=typeName COMMAND_KYWD NCNAME paramsSpec? SEMICOLON
         -> ^(COMMAND_KYWD<CommandDeclarationNode> NCNAME paramsSpec? ^(RETURNS_KYWD ^($rt)))
      )
    )
  ;

// should generate #(LOOKUP_KYWD stateName (returnsSpec)* (paramsSpec)*)

lookupDeclaration : 
    (
      // old style single return syntax
      rt=typeName LOOKUP_KYWD sn=NCNAME paramsSpec? SEMICOLON
      -> ^(LOOKUP_KYWD $sn ^(RETURNS_KYWD ^($rt)) paramsSpec?)
    |
      // multiple return syntax
      LOOKUP_KYWD sn=NCNAME ps=paramsSpec? RETURNS_KYWD rs=returnsSpec SEMICOLON
      -> ^(LOOKUP_KYWD $sn returnsSpec paramsSpec?)
    )
  ;

paramsSpec :
         LPAREN ( paramSpec ( COMMA paramSpec )* )? RPAREN
         -> ^(PARAMETERS paramSpec*)
 ;

returnsSpec :
        paramSpec ( COMMA paramSpec )*
        -> ^(RETURNS_KYWD paramSpec+) ;

paramSpec : typeName^ NCNAME? ;

typeName :
    BOOLEAN_KYWD
  | INTEGER_KYWD
  | REAL_KYWD
  | STRING_KYWD
 ;

libraryActionDeclaration :
    LIBRARY_ACTION_KYWD^ NCNAME libraryInterfaceSpec? SEMICOLON!
;

libraryInterfaceSpec :
    LPAREN ( libraryParamSpec ( COMMA libraryParamSpec )* )? RPAREN
    -> ^(PARAMETERS libraryParamSpec+)
 ;

libraryParamSpec : ( IN_KYWD^ | IN_OUT_KYWD^ ) typeName NCNAME ;

//
// Actions
// 

action
@init { NodeContext actionContext = null; }
 :
    (actionId=NCNAME COLON)?
	{ 
	  // push new naming context
	  actionContext = m_context =
	    new NodeContext(m_context,
						($actionId == null)? m_context.generateChildNodeName() : $actionId.getText());
	}
	rest=baseAction
	{ 
	  // pop out to previous context
	  m_context = m_context.getParentContext(); 
    }
    -> ^(ACTION<ActionNode>[actionContext] $actionId? $rest)
 ;

baseAction : compoundAction | simpleAction | block ; 

compoundAction : forAction | ifAction | onCommandAction | onMessageAction | whileAction ;

// One-liner actions
simpleAction :
    (NCNAME (LBRACKET | EQUALS)) => assignment
  | ((NCNAME LPAREN) | (LPAREN NCNAME)) => commandInvocation SEMICOLON!
  | libraryCall
  | request
  | update
 ;

forAction :
    FOR_KYWD 
    LPAREN typeName NCNAME EQUALS loopvarinit=expression
    SEMICOLON endtest=expression
    SEMICOLON loopvarupdate=expression
    RPAREN
    action
    -> ^(FOR_KYWD ^(VARIABLE_DECLARATION typeName NCNAME $loopvarinit) $endtest $loopvarupdate action)
 ;

ifAction :
    IF_KYWD^ expression action
    (ELSEIF_KYWD expression action)*
    (ELSE_KYWD! action)?
    ENDIF_KYWD!
 ;

onCommandAction : 
  ON_COMMAND_KYWD^ NCNAME LPAREN! paramsSpec? RPAREN! action
 ;
 
incomingParam : typeName NCNAME 
 ;

onMessageAction :
  ON_MESSAGE_KYWD^ expression action
 ;

whileAction :
    WHILE_KYWD^ expression action
 ;

// *** N.B. The supported schema does not require the strict sequencing of
// the elements inside a block, nor does the XML parser.

block : 
    (variant=sequenceVariantKywd LBRACE -> $variant
     | LBRACE -> BLOCK<BlockNode>)
    comment?
    nodeDeclaration*
    nodeAttribute*
    action*
    RBRACE
	-> ^($block comment? nodeDeclaration* nodeAttribute* action*)
;

sequenceVariantKywd : 
    CONCURRENCE_KYWD<BlockNode>
  | UNCHECKED_SEQUENCE_KYWD<BlockNode>
  | TRY_KYWD<BlockNode>
 ;

comment : COMMENT_KYWD^ STRING SEMICOLON! ;

nodeDeclaration :
    interfaceDeclaration
  | variableDeclaration;

nodeAttribute :
    nodeCondition
  | priority
  | resource
  | resourcePriority
  | permissions ;

nodeCondition : conditionKywd^ expression SEMICOLON! ;

conditionKywd :
    END_CONDITION_KYWD<ConditionNode>
  | INVARIANT_CONDITION_KYWD<ConditionNode>
  | POST_CONDITION_KYWD<ConditionNode>
  | PRE_CONDITION_KYWD<ConditionNode>
  | REPEAT_CONDITION_KYWD<ConditionNode>
  | SKIP_CONDITION_KYWD<ConditionNode>
  | START_CONDITION_KYWD<ConditionNode>
 ;

resource :
    RESOURCE_KYWD^ NAME_KYWD! EQUALS! expression
        ( COMMA!
          ( LOWER_BOUND_KYWD EQUALS! expression
          | UPPER_BOUND_KYWD EQUALS! expression
  		  | RELEASE_AT_TERM_KYWD EQUALS! expression
 		  | PRIORITY_KYWD EQUALS! pe=expression
          )
        )+
        SEMICOLON!
 ;

resourcePriority : RESOURCE_PRIORITY_KYWD^ expression SEMICOLON! ;

priority : PRIORITY_KYWD^ INT SEMICOLON! ;

permissions : PERMISSIONS_KYWD^ STRING SEMICOLON! ;

interfaceDeclaration : in | inOut ;

// The rule is apparently that all variables declared in a node 
// are automatically in/out interface vars for all of that node's descendants.
// So this may only be of use in library node definitions.

in : 
    IN_KYWD^ typeName? NCNAME (COMMA! NCNAME)* SEMICOLON!
  ;

inOut :
    IN_OUT_KYWD^ typeName? NCNAME (COMMA! NCNAME)* SEMICOLON!
  ;

variable : NCNAME<VariableNode> ;

variableDeclaration : 
    tn=typeName
    ( (NCNAME LBRACKET) => arrayVariableDecl[$tn.start] 
    | scalarVariableDecl[$tn.start]
    )+
    SEMICOLON
    -> ^(VARIABLE_DECLARATIONS scalarVariableDecl* arrayVariableDecl*)
  ;

scalarVariableDecl[Token typeName] :
    NCNAME ( EQUALS literalScalarValue )?
    -> ^(VARIABLE_DECLARATION {new PlexilTreeNode($typeName)} NCNAME literalScalarValue?)
  ;

arrayVariableDecl[Token typeName] :
    NCNAME LBRACKET INT RBRACKET ( EQUALS literalValue ) ?
	-> ^(ARRAY_VARIABLE_DECLARATION {new PlexilTreeNode($typeName)} NCNAME INT literalValue?)
  ;

literalScalarValue : 
    booleanLiteral | INT | DOUBLE | STRING ;

literalArrayValue :
    HASHPAREN literalScalarValue* RPAREN
    -> ^(ARRAY_LITERAL literalScalarValue*)
  ;

literalValue : literalScalarValue | literalArrayValue ;

booleanLiteral : TRUE_KYWD | FALSE_KYWD ;

realValue : DOUBLE | INT ;

arrayReference :
    variable LBRACKET expression RBRACKET
    -> ^(ARRAY_REF variable expression)
  ;

commandInvocation :
    ( NCNAME -> ^(COMMAND_KYWD NCNAME)
    | LPAREN expression RPAREN -> expression
    )
    LPAREN argumentList? RPAREN
    -> ^(COMMAND<CommandNode> $commandInvocation argumentList?)
 ;

argumentList : 
    argument (COMMA argument)*
    -> ^(ARGUMENT_LIST argument*)
  ;

argument : expression ;

assignment :
    assignmentLHS EQUALS assignmentRHS SEMICOLON
    -> ^(ASSIGNMENT<AssignmentNode> assignmentLHS assignmentRHS)
 ;

assignmentLHS : 
    ( NCNAME LBRACKET )
    => arrayReference
  | 
    variable
;

// *** Note ambiguity in RHS ***
assignmentRHS :
   (NCNAME LPAREN) => commandInvocation
 |
   (LPAREN expression RPAREN LPAREN) => commandInvocation
 |
   expression
 ;

//
// Update nodes
//

update : UPDATE_KYWD^ ( pair ( COMMA! pair )* )? SEMICOLON! ;

//
// Request nodes
//
// Note that the node name need not be known
//

request : REQUEST_KYWD^ NCNAME ( pair ( COMMA! pair )* )? SEMICOLON! ;

// common to both update and request nodes

pair : NCNAME EQUALS! expression ;

//
// Library Call nodes
//

libraryCall :
  LIBRARY_CALL_KYWD^ libraryNodeIdRef ( aliasSpecs )? SEMICOLON! ;

libraryNodeIdRef : NCNAME ;

aliasSpecs :
  LPAREN ( aliasSpec ( COMMA aliasSpec )* )? RPAREN
  -> ^(ALIASES aliasSpec*)
  ;

// must disambiguate
aliasSpec : 
  ( (NCNAME EQUALS NCNAME) )=> varAlias
  | constAlias
 ;

constAlias : 
    nodeParameterName EQUALS literalValue
	-> ^(CONST_ALIAS nodeParameterName literalValue)
  ;

varAlias : 
    nodeParameterName EQUALS variable
    -> ^(VARIABLE_ALIAS nodeParameterName variable)
  ;

nodeParameterName : NCNAME ;

///////////////////
//  EXPRESSIONS  //
///////////////////

//
// Implement operator precedence as in C(++)/Java
// Precedence taken from Harbison & Steele, 1995
//

expression : logicalOr ;

// 1 sequential evaluation (,) - not implemented here
// 2 assignment (=, +=, and friends) - not implemented here
// 3 conditional (? :) - not in the Plexil language

// 4 logical OR

logicalOr :
   ( o1=logicalXOR -> $o1 )
   ( OR_KYWD on=logicalXOR
     -> ^(OR_KYWD $logicalOr $on)
   )*
 ;

// 4.5 logical XOR (not in C)

logicalXOR :
    ( x1=logicalAnd -> $x1 )
    ( XOR_KYWD xn=logicalAnd
      -> ^(XOR_KYWD $logicalXOR $xn)
    )*
 ;

// 5 logical AND

logicalAnd : 
   ( a1=equality -> $a1 )
   ( AND_KYWD an=equality
     -> ^(AND_KYWD $logicalAnd $an)
   )*
 ;

// 6 bitwise OR - not in the Plexil language
// 7 bitwise XOR - not in the Plexil language
// 8 bitwise AND - not in the Plexil language

// 9 equality/inequality

equality :
    ( e1=relational -> $e1)
    ( equalityOp en=relational 
	  -> ^(equalityOp $equality $en)
	)?
 ;

equalityOp :
    DEQUALS
  | NEQUALS
 ;

// 10 relational (<, <=, >, >=)

relational :
    ( a1=additive -> $a1 )
	( relationalOp an=additive
	  -> ^(relationalOp $relational $an)
    )?
 ;

relationalOp : 
    GREATER
  | GEQ
  | LESS
  | LEQ
 ;

// 11 left/right shift - not in the Plexil language

// 12 additive (+, -)

additive :
    ( m1=multiplicative -> $m1 )
    ( addOp mn=multiplicative
	  -> ^(addOp $additive $mn)
	)* 
 ;

addOp : 
    PLUS
  | MINUS
 ;

// 13 multiplicative (*, /, %, mod)

multiplicative :
    ( u1=unary -> $u1 )
    ( multOp un=unary
      -> ^(multOp $multiplicative $un)
    )*
 ;

multOp :
    ASTERISK
  | SLASH
  | PERCENT
 ;

// 14 casts - not in the Plexil language

// 15 indirection (*), address of(&), sizeof, preincrement/decrement (++x, --x)
//    - not in the Plexil language

// 15 prefix unary - arithmetic negation, plus, logical not

unary :
    unaryOp^ quantity
    | quantity
 ;

unaryOp : 
    PLUS
  | MINUS
  | NOT_KYWD
 ;

// 17 postincrement/decrement (x++, x--), indirect selection (->), function call
// 	  - not in the Plexil language

// 17 Subscripting, direct selection, simple tokens

quantity :
    LPAREN! expression RPAREN!
  | BAR expression BAR -> ^(ABS_KYWD expression)
  | oneArgFn^ LPAREN! expression RPAREN!
  | isKnownExp
  | lookupExpr
  | messageReceivedExp
  | nodeStatePredicateExp
  | (NCNAME PERIOD COMMAND_HANDLE_KYWD) => nodeCommandHandleVariable
  | (NCNAME PERIOD FAILURE_KYWD) => nodeFailureVariable
  | (NCNAME PERIOD OUTCOME_KYWD) => nodeOutcomeVariable
  | (NCNAME PERIOD STATE_KYWD) => nodeStateVariable
  | (NCNAME PERIOD nodeStateKywd) => nodeTimepointValue
  | (NCNAME LBRACKET) => arrayReference
  | variable
  | literalValue
  | nodeCommandHandleKywd
  | nodeFailureKywd
  | nodeStateKywd
  | nodeOutcomeKywd
 ;

// can add more later
oneArgFn : 
    SQRT_KYWD
  | ABS_KYWD
 ;

isKnownExp :
   IS_KNOWN_KYWD^ 
   LPAREN!
   ( 
     (NCNAME PERIOD STATE_KYWD) => nodeStateVariable
   | (NCNAME PERIOD OUTCOME_KYWD) => nodeOutcomeVariable
   | (NCNAME PERIOD FAILURE_KYWD) => nodeFailureVariable
   | (NCNAME PERIOD nodeStateKywd) => nodeTimepointValue
   | (NCNAME LBRACKET) => arrayReference
   | variable
   )
   RPAREN! ;

nodeStatePredicate :
    NODE_EXECUTING_KYWD
  | NODE_FAILED_KYWD
  | NODE_FINISHED_KYWD
  | NODE_INACTIVE_KYWD
  | NODE_INVARIANT_FAILED_KYWD
  | NODE_ITERATION_ENDED_KYWD
  | NODE_ITERATION_FAILED_KYWD
  | NODE_ITERATION_SUCCEEDED_KYWD
  | NODE_PARENT_FAILED_KYWD
  | NODE_POSTCONDITION_FAILED_KYWD
  | NODE_PRECONDITION_FAILED_KYWD
  | NODE_SKIPPED_KYWD
  | NODE_SUCCEEDED_KYWD
  | NODE_WAITING_KYWD
 ;

nodeStatePredicateExp : nodeStatePredicate^ LPAREN! NCNAME RPAREN! ;

nodeStateKywd : 
     EXECUTING_STATE_KYWD
   | FAILING_STATE_KYWD
   | FINISHED_STATE_KYWD
   | FINISHING_STATE_KYWD
   | INACTIVE_STATE_KYWD
   | ITERATION_ENDED_STATE_KYWD
   | WAITING_STATE_KYWD
 ;

messageReceivedExp :
  MESSAGE_RECEIVED_KYWD^ LPAREN! STRING RPAREN!
 ;

// *** Want nodeStateKywd to turn into a LiteralNode but can't figure out how
nodeState : nodeStateVariable | nodeStateKywd ;

nodeStateVariable : NCNAME PERIOD! STATE_KYWD^ ;

// *** Want nodeOutcomeKywd to turn into a LiteralNode but can't figure out how
nodeOutcome : nodeOutcomeVariable | nodeOutcomeKywd ;

nodeOutcomeVariable : NCNAME PERIOD! OUTCOME_KYWD^ ;

nodeOutcomeKywd :
    SUCCESS_OUTCOME_KYWD
  | FAILURE_OUTCOME_KYWD
  | SKIPPED_OUTCOME_KYWD
;

// *** Want nodeCommandHandleKywd to turn into a LiteralNode but can't figure out how
nodeCommandHandle : nodeCommandHandleVariable | nodeCommandHandleKywd ;

nodeCommandHandleVariable : NCNAME PERIOD! COMMAND_HANDLE_KYWD^ ;

nodeCommandHandleKywd :
    COMMAND_ABORTED_KYWD
  | COMMAND_ABORT_FAILED_KYWD
  | COMMAND_ACCEPTED_KYWD
  | COMMAND_DENIED_KYWD
  | COMMAND_FAILED_KYWD
  | COMMAND_RCVD_KYWD
  | COMMAND_SENT_KYWD
  | COMMAND_SUCCESS_KYWD
 ;

// *** Want nodeFailureKywd to turn into a LiteralNode but can't figure out how
nodeFailure : nodeFailureVariable | nodeFailureKywd ;

nodeFailureVariable : NCNAME PERIOD! FAILURE_KYWD^ ;

nodeFailureKywd :
    PRE_CONDITION_FAILED_KYWD
  | POST_CONDITION_FAILED_KYWD
  | INVARIANT_CONDITION_FAILED_KYWD
  | PARENT_FAILED_KYWD
 ;

nodeTimepointValue :
   NCNAME PERIOD nodeStateKywd PERIOD timepoint
   -> ^(NODE_TIMEPOINT_VALUE NCNAME nodeStateKywd timepoint)
 ;

timepoint : START_KYWD | END_KYWD ;

//
// Lookups
//

lookupExpr : lookupOnChange | lookupNow | lookup ;

// should produce an AST of the form
// #(LOOKUP_ON_CHANGE_KYWD lookupInvocation (tolerance)? )
// N.b. tolerance is optional

lookupOnChange :
       LOOKUP_ON_CHANGE_KYWD^ LPAREN! lookupInvocation (COMMA! tolerance)? RPAREN!
;

tolerance : realValue | variable ;

// should produce an AST of the form
// #(LOOKUP_NOW_KYWD stateNameExp (argumentList)? )

lookupNow :
    LOOKUP_NOW_KYWD^ LPAREN! lookupInvocation RPAREN! ;

// new generic lookup
// should produce an AST of the form
// #(LOOKUP_KYWD lookupInvocation (tolerance)? )
// N.b. tolerance is optional

lookup :
    LOOKUP_KYWD^ LPAREN! lookupInvocation (COMMA! tolerance)? RPAREN!
  ;

lookupInvocation :
  ( stateName
    |
    ( LPAREN! stateNameExp RPAREN! )
  )
  ( LPAREN! (argumentList)? RPAREN! )?
 ;

stateName :
    NCNAME -> ^(STATE_NAME NCNAME)
 ;

stateNameExp : expression ;


//
// ****************************************
// ********* BEGIN LEXER GRAMMAR **********
// ****************************************
//

//
// Tokens with synonyms
//

// Conditions
START_CONDITION_KYWD : 'StartCondition' | 'Start' ;
REPEAT_CONDITION_KYWD : 'RepeatCondition' | 'Repeat' ;
SKIP_CONDITION_KYWD : 'SkipCondition' | 'Skip' ;
PRE_CONDITION_KYWD : 'PreCondition' | 'Pre' ;
POST_CONDITION_KYWD : 'PostCondition' | 'Post';
INVARIANT_CONDITION_KYWD : 'InvariantCondition' | 'Invariant';
END_CONDITION_KYWD : 'EndCondition' | 'End' ;

// Logical operators
AND_KYWD : 'AND' | '&&' ;
OR_KYWD : 'OR' | '||' ;
NOT_KYWD : 'NOT' | '!' ;

STRING: '"' (Escape|~('"'|'\\'))* '"'
      | '\'' (Escape|~('\''|'\\'))* '\''
      ;
fragment Escape:
  '\\'
  ('n' | 't' | 'b' | 'f' |'\n' | '\r' | '"' | '\'' | '\\' | UnicodeEscape | OctalEscape);

fragment UnicodeEscape: 
  'u' HexDigit HexDigit HexDigit HexDigit;

fragment OctalEscape: 
  QuadDigit ( OctalDigit OctalDigit? )?
  | OctalDigit OctalDigit? ;

fragment QuadDigit: ('0'..'3') ;
fragment OctalDigit: ('0'..'7') ;
fragment Digit: ('0'..'9') ;
fragment HexDigit: (Digit|'A'..'F'|'a'..'f') ;

// a few fragment methods to assist in matching floating point numbers
fragment Exponent:  ('e'|'E') (PLUS | MINUS)? (Digit)+ ;

PLUS : '+';
MINUS : '-';
PERIOD : '.';

// the following returns tokens DOUBLE, INT

// *** Chuck's notes:
// prefixes - 
//  +/- is always decimal, can be either int or double
//  0x is always hexadecimal (must not be signed, no decimal point)
//  0o is always octal (must not be signed, no decimal point)
//  0b is always binary (must not be signed, no decimal point)
//  . is always double (decimal)

// *** TODO: set base for non-decimal INTs ***

INT_OR_DOUBLE
@init { int base = 10; } :
   (
    ( '0' { $type = INT; } // special case for just '0'
     ( ( ('x'|'X') { base = 16; } // hex
         (HexDigit)+
         )
       |
       ( ('o'|'O') { base = 8; } // octal
         (OctalDigit)+
       )
       |
       ( ('b'|'B') { base = 2; } // binary
         ('0'|'1')+   
       )
       |
       ( PERIOD { $type = DOUBLE; }
         Digit* Exponent?
       )
       |
       ( Exponent { $type = DOUBLE; } )
     )?
   )
   | 
   ( (PLUS | MINUS)? PERIOD ) =>
   ( (PLUS | MINUS)?
     PERIOD { $type = DOUBLE; }
     Digit+ Exponent? 
   )
   |
   ( (PLUS | MINUS)?
     Digit+ { $type = INT; } 
     (PERIOD (Digit)* { $type = DOUBLE; } )?
     (Exponent { $type = DOUBLE; } )?
   )
   )
 ;

// XML spec says:
// NCName ::= (Letter | '_') (NCNameChar)*
// NCNameChar ::= Letter | Digit | '.' | '-' | '_' | CombiningChar | Extender
// (see http://www.w3.org/TR/xml/ for definitions of Letter, Digit, CombiningChar, and Extender)

// *** ensure this does not conflict with keywords! ***

NCNAME :
    (Letter|'_') (Letter|Digit|PERIOD|MINUS|'_')*
  ;

fragment Letter : 'a'..'z'|'A'..'Z' ;

// Whitespace -- ignored
WS :
    ( ' ' | '\t' | '\f' | '\n' | '\r' )+
    { $channel = HIDDEN; }
  ;

// Single-line comments
SL_COMMENT :
    ( '//' ~('\r' | '\n')* ('\r\n' | '\r' | '\n')
      { $channel = HIDDEN; }
	| '//' ~('\r' | '\n')*
      { $channel = HIDDEN; }
    )
  ;

// multiple-line comments
ML_COMMENT :
    '/*' ( options {greedy=false;} : . )* '*/'
    { $channel=HIDDEN; }
  ;
