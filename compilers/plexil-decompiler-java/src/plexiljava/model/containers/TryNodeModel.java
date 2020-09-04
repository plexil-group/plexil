package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;

public class TryNodeModel extends NodeModel {

    public TryNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(getQuality("NodeId").getValue(), ": {\n");
        indentLevel++;

        if( hasQuality("Priority") ) {
            dsb.addIndent(indentLevel+1);
            dsb.addLine("Priority: ", getQuality("Priority").getValue(), ";");
        }

        dsb.addIndent(indentLevel);
        dsb.append("Try\n");
        dsb.addIndent(indentLevel);
        dsb.append("{\n");
        if( hasQuality("Priority") ) {
            dsb.addIndent(indentLevel+1);
            dsb.addLine("Priority: ", getQuality("Priority").getValue(), ";");
        }
        for( BaseModel child : children ) {
            if( child instanceof ConditionModel ) {
                continue;
            }
            dsb.append(child.decompile(indentLevel+1), "\n");
        }
        dsb.addBlockCloser(indentLevel);
        dsb.append("\n");
        indentLevel--;
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
