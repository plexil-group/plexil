package plexiljava.decompilation;

import plexiljava.main.Constants;

public class DecompilableStringBuilder {

	public StringBuilder sb;
	
	public DecompilableStringBuilder() {
		sb = new StringBuilder();
	}
	
	public DecompilableStringBuilder(StringBuilder sb) {
		sb = new StringBuilder(sb);
	}
	
	public DecompilableStringBuilder append(Object o) {
		sb.append(o);
		return this;
	}
	
	public DecompilableStringBuilder append(Object...objects) {
		for( Object o : objects ) {
			sb.append(o);
		}
		return this;
	}
	
	public DecompilableStringBuilder addIndent(int indentLevel) {
		for( int f=0; f<indentLevel; f++ ) {
			sb.append(Constants.INDENT);
		}
		return this;
	}
	
	public DecompilableStringBuilder addLine(String line) {
		sb.append(line);
		sb.append("\n");
		return this;
	}
	
	public DecompilableStringBuilder addBlockOpener(String blockType) {
		sb.append(": ");
		sb.append(blockType);
		sb.append(" {\n");
		return this;
	}
	
	public DecompilableStringBuilder addBlockOpener() {
		sb.append(":\n{\n");
		return this;
	}
	
	public DecompilableStringBuilder addBlockCloser(int indentLevel) {
		addIndent(indentLevel);
		sb.append("}");
		return this;
	}
	
	public DecompilableStringBuilder addReference(String reference) {
		sb.append("@");
		sb.append(reference);
		return this;
	}
	
	@Override
	public String toString() {
		return sb.toString();
	}
}
