
//
// ScriptElementStateAST
//

package plexilscript;

public class ScriptElementStateAST implements ScriptElementAST {
    StateAST state;

    public ScriptElementStateAST (StateAST state0) {
	state = state0;
    }

    public void print () {
	state.print ();
    }
}

