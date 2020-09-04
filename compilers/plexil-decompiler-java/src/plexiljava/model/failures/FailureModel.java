package plexiljava.model.failures;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class FailureModel extends TypedNodeModel {

    public FailureModel(BaseModel node) {
        super(node);
    }

    public FailureModel(BaseModel node, String type) {
        super(node, type);
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeRef") || hasQuality("NodeId");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( hasQuality("NodeRef") ) {
            dsb.append(getQuality("NodeRef").getValue(), ".failure == ", type);
        } else {
            dsb.append(getQuality("NodeId").getValue(), ".failure == ", type);
        }
        return dsb.toString();
    }

}