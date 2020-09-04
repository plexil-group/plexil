package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class PlusInfinityModel extends NodeModel {

    public PlusInfinityModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("INF");

        return dsb.toString();
    }

}
