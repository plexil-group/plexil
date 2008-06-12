
//
// CommandAST
//

package plexilscript;

import java.util.List;

public class CommandAST implements ElementAST {
    String             name;
    List<ParameterAST> parameters;
    List<String>       results;

    public CommandAST (String name0, List<ParameterAST> parameters0, List<String> results0) {
	name       = name0;
	parameters = parameters0;
	results    = results0;
    }

    public void print () {
	System.out.printf ("    <Command name=\"%s\" type=\"string\">\n", name);
	for (ParameterAST p : parameters)
	    p.print ();
	for (String r : results)
	    System.out.printf ("      <Result>\"%s\"</Result>\n", r);
	System.out.printf ("    </Command>\n");
    }
}

