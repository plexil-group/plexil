package plexiljava.decompilation;

public class DecompilableStringBuilder {

    public StringBuilder sb;

    public static final String REFERENCE_IDENTIFIER = "__@__";

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
            sb.append("  ");
        }
        return this;
    }

    public DecompilableStringBuilder addLine(Object...objects) {
        for( Object o : objects ) {
            sb.append(o);
        }
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
        sb.append(": {\n");
        return this;
    }

    public DecompilableStringBuilder addBlockCloser(int indentLevel) {
        addIndent(indentLevel);
        sb.append("}");
        return this;
    }

    public DecompilableStringBuilder addReference(String reference) {
        sb.append(REFERENCE_IDENTIFIER);
        sb.append(reference);
        return this;
    }

    @Override
    public String toString() {
        return sb.toString();
    }
}
