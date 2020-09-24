package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class StringValueModel extends NodeModel {

    public StringValueModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("\"\"");
        return dsb.toString();
    }

}
