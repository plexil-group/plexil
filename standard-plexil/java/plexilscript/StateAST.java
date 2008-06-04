
//
// StateAST
//

package plexilscript;

public class StateAST {
    String name;
    String type;
    String value;

    public StateAST (String name0, String type0, String value0) {
	name  = name0;
	type  = type0;
	value = value0;
    }

    public void print () {
	System.out.printf ("    <State name=\"%s\" type=\"%s\">\n", name, type);
	System.out.printf ("      <Value>%s</Value>\n", value);
	System.out.printf ("    </State>\n");
    }
}

