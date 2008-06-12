
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
        ast = initialState |
        ast = script       |
        ast = state        |
        ast = updateAck    |
        ast = command      |
        ast = functionCall |
        ast = commandAck   |
        ast = commandAbort |
        ast = simultaneous
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

state returns [StateAST ast = null] :
        { List<String> vs; }
        "state" n:ID COLON t:ID EQUALS vs = values SEMI
        { ast = new StateAST (n.getText(), t.getText(), vs); }
    ;

updateAck returns [UpdateAckAST ast = null] :
        "update-ack" n:ID SEMI
        { ast = new UpdateAckAST (n.getText()); }
    ;

functionCall returns [FunctionCallAST ast = null] :
        { List<ParameterAST> ps; List<String> vs; }
        "function-call" n:ID ps = parameters EQUALS vs = values SEMI
        { ast = new FunctionCallAST (n.getText(), ps, vs); }
    ;

command returns [CommandAST ast = null] :
        { List<ParameterAST> ps; List<String> vs; }
        "command" n:ID ps = parameters EQUALS vs = values SEMI
        { ast = new CommandAST (n.getText(), ps, vs); }
    ;

commandAck returns [CommandAckAST ast = null] :
        { List<ParameterAST> ps; List<String> vs; }
        "command-ack" n:ID ps = parameters EQUALS vs = values SEMI
        { ast = new CommandAckAST (n.getText(), ps, vs); }
    ;

commandAbort returns [CommandAbortAST ast = null] :
        { List<ParameterAST> ps; List<String> vs; }
        "command-abort" n:ID ps = parameters EQUALS vs = values SEMI
        { ast = new CommandAbortAST (n.getText(), ps, vs); }
    ;

parameters returns [List<ParameterAST> ast = null] :
        { ParameterAST p;
          ast = new LinkedList<ParameterAST> (); }
        LPAREN (p = parameter { ast.add (p); }
            (COMMA p = parameter { ast.add (p); })* )? RPAREN
    ;

parameter returns [ParameterAST ast = null] :
        v:ID COLON t:ID
        { ast = new ParameterAST (v.getText(), t.getText()); }
    ;

values returns [List<String> ast = null] :
        { ast = new LinkedList<String> (); }
        (v1:ID { ast.add (v1.getText()); }) |
        (LPAREN v2:ID { ast.add (v2.getText()); }
              (COMMA v3:ID { ast.add (v3.getText()); })* RPAREN)
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

ID      : ('A'..'Z'|'a'..'z'|'0'..'9'|'_'|'-'|'.'|'\"')+ ;
WS      : (' '|'\t'|'\n' { newline(); } |'\r')+ { $setType(Token.SKIP); } ;
COMMENT : "//" (~'\n')* '\n' { newline(); $setType(Token.SKIP); } ;

