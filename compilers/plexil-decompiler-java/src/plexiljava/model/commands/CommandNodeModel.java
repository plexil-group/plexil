package plexiljava.model.commands;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class CommandNodeModel extends NodeModel {

    public CommandNodeModel(BaseModel node) {
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
        for( BaseModel child : children ) {
            dsb.addLine(child.decompile(indentLevel+1));
        }
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
