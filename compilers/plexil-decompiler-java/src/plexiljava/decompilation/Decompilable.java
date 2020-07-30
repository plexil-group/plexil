package plexiljava.decompilation;

import plexiljava.main.Constants;

public interface Decompilable {
	public default String indent(int indentLevel) {
		String ret = "";
		for( int f=0; f<indentLevel; f++ ) {
			ret += Constants.INDENT;
		}
		return ret;
	}
	
	public String decompile(int indentLevel); 
}
