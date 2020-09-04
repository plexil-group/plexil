package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NodeStateVariableModel extends NodeModel {

    public NodeStateVariableModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeRef") || hasQuality("NodeId");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(hasQuality("NodeRef") ? getQuality("NodeRef").getValue() : getQuality("NodeId").getValue(), ".state");
        return dsb.toString();
    }
}
