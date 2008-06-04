
//
// CommandAckAST
//

package plexilscript;

import java.util.List;

public class CommandAckAST {
    String name;
    List<ParameterAST> parameters;

    public CommandAckAST (String name0, List<ParameterAST> parameters0) {
	name       = name0;
	parameters = parameters0;
    }

    public void print () {
	System.out.printf ("    <CommandAck name=\"%s\" type=\"string\">\n", name);
	for (ParameterAST p : parameters)
	    p.print ();
	System.out.printf ("      <Result>\"COMMAND_SUCCESS\"</Result>\n");
	System.out.printf ("    </CommandAck>\n");
    }
}

