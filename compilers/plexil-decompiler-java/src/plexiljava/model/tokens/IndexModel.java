package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class IndexModel extends NodeModel {

    public IndexModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !qualities.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(qualities.get(0).getValue());
        return dsb.toString();
    }

}
