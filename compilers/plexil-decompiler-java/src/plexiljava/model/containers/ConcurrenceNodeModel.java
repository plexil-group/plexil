package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ConcurrenceNodeModel extends NodeModel {

    public ConcurrenceNodeModel(BaseModel node) {
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
        dsb.append(getQuality("NodeId").getValue());
        dsb.addBlockOpener("Concurrence");
        if( hasQuality("Priority") ) {
            dsb.addIndent(indentLevel+1);
            dsb.addLine("Priority: ", getQuality("Priority").getValue(), ";");
        }
        for( BaseModel child : children ) {
            dsb.addLine(child.decompile(indentLevel+1));
        }
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
