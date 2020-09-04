package plexiljava.model.commandhandles;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class CommandHandleModel extends TypedNodeModel {

    public CommandHandleModel(BaseModel node) {
        super(node);
    }

    public CommandHandleModel(BaseModel node, String type) {
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
            dsb.append(getQuality("NodeRef").getValue(), ".command_handle == ", type);
        } else {
            dsb.append(getQuality("NodeId").getValue(), ".command_handle == ", type);
        }
        return dsb.toString();
    }

}