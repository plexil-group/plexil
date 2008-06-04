
//
// PlexilScriptAST
//

package plexilscript;

public class PlexilScriptAST {
    InitialStateAST initialState;
    ScriptAST       script;

    public PlexilScriptAST (InitialStateAST initialState0,  ScriptAST script0) {
	initialState = initialState0;
	script       =  script0;
    }

    public void print () {
	System.out.printf ("<PLEXILScript>\n");
	initialState.print ();
	System.out.printf ("\n");
	script.print ();
	System.out.printf ("</PLEXILScript>\n");
    }
}

