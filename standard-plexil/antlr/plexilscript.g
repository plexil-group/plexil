
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
        { InitialStateAST i; ScriptAST s; }
        i = initialState
        s = script
        { ast = new PlexilScriptAST (i, s); }
    ;

initialState returns [InitialStateAST ast = null] :
        { List<StateAST> es; }
        "initial-state" LBRACE
        es = initialStateElements
        RBRACE
        { ast = new InitialStateAST (es); }
    ;

initialStateElements returns [List<StateAST> ast = null] :
        { StateAST e;
          ast = new LinkedList<StateAST> (); }
        ( e = state { ast.add (e); } )*
    ;

script returns [ScriptAST ast = null] :
        { List<ScriptElementAST> es; }
        "script" LBRACE
        es = scriptElements
        RBRACE
        { ast = new ScriptAST (es); }
    ;

scriptElements returns [List<ScriptElementAST> ast = null] :
        { ScriptElementAST e;
          ast = new LinkedList<ScriptElementAST> (); }
        ( e = scriptElement { ast.add (e); } )*
    ;

scriptElement returns [ScriptElementAST ast = null] :
        ( { StateAST e1; } 
            e1 = state
          { ast = new ScriptElementStateAST (e1); }) |
        ( { CommandAckAST e2; }
            e2 = commandAck
          { ast = new ScriptElementCommandAckAST (e2); })
    ;

state returns [StateAST ast = null] :
        "state" n:ID COLON t:ID EQUALS v:ID SEMI
        { ast = new StateAST (n.getText(), t.getText(), v.getText()); }
    ;

commandAck returns [CommandAckAST ast = null] :
        { List<ParameterAST> ps; }
        "command-ack" n:ID ps = parameters SEMI
        { ast = new CommandAckAST (n.getText(), ps); }
    ;

parameters returns [List<ParameterAST> ast = null] :
        { ParameterAST p;
          ast = new LinkedList<ParameterAST> (); }
        (LPAREN (p = parameter { ast.add (p); }
              (COMMA p = parameter { ast.add (p); })* )? RPAREN)?
    ;

parameter returns [ParameterAST ast = null] :
        v:ID COLON t:ID
        { ast = new ParameterAST (v.getText(), t.getText()); }
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

