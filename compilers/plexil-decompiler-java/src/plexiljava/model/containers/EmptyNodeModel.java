package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class EmptyNodeModel extends NodeModel {

    public EmptyNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeId");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(getQuality("NodeId").getValue(), ": Concurrence {");
        if( !children.isEmpty() ) {
            dsb.addLine("");
        }
        if( hasQuality("Priority") ) {
            dsb.addIndent(indentLevel+1);
            dsb.addLine("Priority: ", getQuality("Priority").getValue(), ";");
        }
        for( BaseModel child : children ) {
            dsb.addLine(child.decompile(indentLevel+1));
        }
        if( !children.isEmpty() ) {
            dsb.addBlockCloser(indentLevel);
        } else {
            dsb.append("}");
        }
        return dsb.toString();
    }

}
