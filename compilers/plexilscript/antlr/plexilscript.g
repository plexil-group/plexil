// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

//
// Standard Plexil Script Parser
//
// Works with Antlr 2.7.6.
// Doesn't work with Antlr 3.0.1.
//

header {
  package plexilscript;
  import java.util.List;
  import java.util.LinkedList;
}

class PlexilScriptParser extends Parser;

plexilScript returns [PlexilScriptAST ast = null] :
        { List<ElementAST> es; }
        es = elements
        { ast = new PlexilScriptAST (es); }
    ;

elements returns [List<ElementAST> ast = null] :
        { ElementAST e;
          ast = new LinkedList<ElementAST> (); }
        ( e = element { ast.add (e); } )*
    ;

element returns [ElementAST ast = null] :
      ast = initialState
    | ast = script      
    | ast = state       
    | ast = updateAck   
    | ast = command     
    | ast = functionCall
    | ast = commandAbort
    | ast = simultaneous
    | ast = commandAck  
    | ast = commandAccepted
    | ast = commandDenied
    | ast = commandSentToSystem
    | ast = commandRcvdBySystem
    | ast = commandSuccess
    | ast = commandFailed
    | ast = delay
    ;

initialState returns [InitialStateAST ast = null] :
        { List<ElementAST> es; }
        "initial-state" LBRACE
        es = elements
        RBRACE
        { ast = new InitialStateAST (es); }
    ;

simultaneous returns [SimultaneousAST ast = null] :
        { List<ElementAST> es; }
        "simultaneous" LBRACE
        es = elements
        RBRACE
        { ast = new SimultaneousAST (es); }
    ;

script returns [ScriptAST ast = null] :
        { List<ElementAST> es; }
        "script" LBRACE
        es = elements
        RBRACE
        { ast = new ScriptAST (es); }
    ;

updateAck returns [UpdateAckAST ast = null] :
        "update-ack" n:ID SEMI
        { ast = new UpdateAckAST (n.getText()); }
    ;

state returns [CommandAST ast = null] :
        { List<ParameterAST> ps; String t; List<String> vs; }
        "state" n:ID ps = parameters EQUALS vs = values COLON t = type SEMI
        { ast = new CommandAST ("State", "Value", n.getText(), ps, t, vs); }
    ;

functionCall returns [CommandAST ast = null] :
        { List<ParameterAST> ps; String t; List<String> vs; }
        "function-call" n:ID ps = parameters EQUALS vs = values COLON t = type SEMI
        { ast = new CommandAST ("FunctionCall", "Result", n.getText(), ps, t, vs); }
    ;

command returns [CommandAST ast = null] :
        { List<ParameterAST> ps; String t; List<String> vs; }
        "command" n:ID ps = parameters EQUALS vs = values COLON t = type SEMI
        { ast = new CommandAST ("Command", "Result", n.getText(), ps, t, vs); }
    ;

commandAbort returns [CommandAST ast = null] :
        { List<ParameterAST> ps; String t; List<String> vs; }
        "command-abort" n:ID ps = parameters EQUALS vs = values COLON t = type SEMI
        { ast = new CommandAST ("CommandAbort", "Result", n.getText(), ps, t, vs); }
    ;

commandAck returns [CommandAST ast = null] :
        { List<ParameterAST> ps; String t; List<String> vs; }
        "command-ack" n:ID ps = parameters EQUALS vs = values COLON t = type SEMI
        { ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, t, vs); }
    ;

commandAccepted returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-accepted" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_ACCEPTED");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

commandDenied returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-denied" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_DENIED");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

commandSentToSystem returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-sent-to-system" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_SENT_TO_SYSTEM");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

commandRcvdBySystem returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-rcvd-by-system" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_RCVD_BY_SYSTEM");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

commandSuccess returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-success" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_SUCCESS");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

commandFailed returns [CommandAST ast = null] :
        { List<ParameterAST> ps; }
        "command-failed" n:ID ps = parameters SEMI
        { List<String> vs = new LinkedList<String> (); vs.add ("COMMAND_FAILED");
          ast = new CommandAST ("CommandAck", "Result", n.getText(), ps, "string", vs); }
    ;

delay returns [DelayAST ast = null] :
        "delay" SEMI
        { ast = new DelayAST(); }
    ;

parameters returns [List<ParameterAST> ast = null] :
        { ParameterAST p;
          ast = new LinkedList<ParameterAST> (); }
        LPAREN (p = parameter { ast.add (p); }
            (COMMA p = parameter { ast.add (p); })* )? RPAREN
    ;

parameter returns [ParameterAST ast = null] :
        { String v, t; }
        v = value COLON t = type
        { ast = new ParameterAST (v, t); }
    ;

values returns [List<String> ast = null] :
        { String v; ast = new LinkedList<String> (); }
        ((v = value { ast.add (v); })
        | (LPAREN v = value { ast.add (v); }
                (COMMA v = value { ast.add (v); })* RPAREN))
    ;

value returns [String ast = null] :
      "true"   { ast = "true"; }
    | "false"  { ast = "false"; }
    | u:UNKNOWN { ast = "Plexil_Unknown"; }
    | s:STRING { ast = s.getText(); }
    | n:NUMBER { ast = n.getText(); }
    ;

type returns [String ast = null] :
      "int-array"    { ast = "int-array"; }
    | "string-array" { ast = "string-array"; }
    | "real-array"   { ast = "real-array"; }
    | "string"       { ast = "string"; }
    | "bool-array"   { ast = "bool-array"; }
    | "int"          { ast = "int"; }
    | "real"         { ast = "real"; }
    | "bool"         { ast = "bool"; }
    ;

class PlexilScriptLexer extends Lexer;

LBRACE : '{';
RBRACE : '}';
LPAREN : '(';
RPAREN : ')';
SEMI   : ';';
COMMA  : ',';
COLON  : ':';
EQUALS : '=';


protected LETTER : 'A'..'Z' | 'a'..'z' ;
protected DIGIT  : '0'..'9' ;

UNKNOWN : '<' 'u' 'n' 'k' 'n' 'o' 'w' 'n' '>' ;
STRING : '"'! (~'"')* '"'! ;
NUMBER : ('-')? (DIGIT)+ ('.' (DIGIT)+)? ;
ID      : LETTER (LETTER|DIGIT|'_'|'-'|'.')* ;
WS      : (' '|'\t'|'\n' { newline(); } |'\r')+ { $setType(Token.SKIP); } ;
COMMENT : "//" (~'\n')* '\n' { newline(); $setType(Token.SKIP); } ;

// ID      : ('A'..'Z'|'a'..'z'|'0'..'9'|'_'|'-'|'.'|'\"')+ ;
// WS      : (' '|'\t'|'\n' { newline(); } |'\r')+ { $setType(Token.SKIP); } ;
// COMMENT : "//" (~'\n')* '\n' { newline(); $setType(Token.SKIP); } ;

