package plexiljava.model.expressions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NodeTimepointValue extends NodeModel {

    public NodeTimepointValue(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeId") && hasQuality("NodeStateValue") && hasQuality("Timepoint");
    }

    @Override
    public String translate(int indentLevel) {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        dsb.append(getQuality("NodeId").getValue(), ".", getQuality("NodeStateValue").getValue(), ".", getQuality("Timepoint").getValue());
        return dsb.toString();
    }
}
