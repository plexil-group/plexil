package plexiljava.decompilation;

import plexiljava.model.BaseModel.PatternRecognitionFailureException;

public interface Decompilable {
	
	public boolean verify();
	
	public String translate(int indentLevel) throws PatternRecognitionFailureException;
	
	public String decompile(int indentLevel) throws PatternRecognitionFailureException; 
	
}
