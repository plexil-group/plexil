
//
// ScriptAST
//

package plexilscript;

import java.util.List;

public class ScriptAST {    
    List<ScriptElementAST> elements;

    public ScriptAST (List<ScriptElementAST> elements0) {
	elements = elements0;
    }

    public void print () {
	System.out.printf ("  <Script>\n");
	for (ScriptElementAST e : elements)
	    e.print ();
	System.out.printf ("  </Script>\n");
    }
}

