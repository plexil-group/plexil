package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ElseNodeModel extends NodeModel {

    public ElseNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("else {\n");
        return dsb.toString();
    }

}
