
//
// ScriptElementCommandAckAST
//

package plexilscript;

public class ScriptElementCommandAckAST implements ScriptElementAST {
    CommandAckAST commandAck;

    public ScriptElementCommandAckAST (CommandAckAST commandAck0) {
	commandAck = commandAck0;
    }	

    public void print () {
	commandAck.print ();
    }
}

