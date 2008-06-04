
//
// InitialStateAST
//

package plexilscript;

import java.util.List;

public class InitialStateAST {
    List<StateAST> elements;

    public InitialStateAST (List<StateAST> elements0) {
	elements = elements0;
    }

    public void print () {
	System.out.printf ("  <InitialState>\n");
	for (StateAST e : elements)
	    e.print ();
	System.out.printf ("  </InitialState>\n");
    }
}

