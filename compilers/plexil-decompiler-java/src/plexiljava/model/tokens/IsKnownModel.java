package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class IsKnownModel extends NodeModel {

    public IsKnownModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !qualities.isEmpty() || !children.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("isKnown(", qualities.isEmpty() ? children.get(0).decompile(0) : qualities.get(0).getValue(), ")");
        return dsb.toString();
    }

}
