package plexiljava.decompilation;

public interface Decompilable {
	
	public boolean verify();
	
	public String decompile(int indentLevel); 
	
}
