package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class InOutModel extends NodeModel {

    public InOutModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !children.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        dsb.append("InOut ", children.get(0).decompile(indentLevel));
        dsb.sb.deleteCharAt(dsb.sb.length()-1);
        if( indentLevel != 0 ) {
            dsb.append(";");
        }
        return dsb.toString();
    }
}
