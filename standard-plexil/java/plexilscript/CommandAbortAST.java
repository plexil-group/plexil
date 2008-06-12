
//
// CommandAbortAST
//

package plexilscript;

import java.util.List;

public class CommandAbortAST implements ElementAST {
    String             name;
    List<ParameterAST> parameters;
    List<String>       results;

    public CommandAbortAST (String name0, List<ParameterAST> parameters0, List<String> results0) {
	name       = name0;
	parameters = parameters0;
	results    = results0;
    }

    public void print () {
	System.out.printf ("    <CommandAbort name=\"%s\" type=\"string\">\n", name);
	for (ParameterAST p : parameters)
	    p.print ();
	for (String r : results)
	    System.out.printf ("      <Result>\"%s\"</Result>\n", r);
	System.out.printf ("    </CommandAbort>\n");
    }
}

