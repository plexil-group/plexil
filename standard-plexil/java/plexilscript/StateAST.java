
//
// StateAST
//

package plexilscript;

import java.util.List;

public class StateAST implements ElementAST {
    String       name;
    String       type;
    List<String> values;

    public StateAST (String name0, String type0, List<String> values0) {
	name   = name0;
	type   = type0;
	values = values0;
    }

    public void print () {
	System.out.printf ("    <State name=\"%s\" type=\"%s\">\n", name, type);
	for (String v : values)
	    System.out.printf ("      <Value>%s</Value>\n", v);
	System.out.printf ("    </State>\n");
    }
}

