package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.ReferringNodeModel;
import plexiljava.model.conditions.ConditionNodeModel;
import plexiljava.model.conditions.RepeatConditionModel;

public class WhileNodeModel extends NodeModel {

    public WhileNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(RepeatConditionModel.class);
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
        dsb.append("while( ");

        RepeatConditionModel repeatCondition = (RepeatConditionModel) getChild(RepeatConditionModel.class);
        String conditionText = repeatCondition.decompile(0);
        for( BaseModel grandchild : repeatCondition.getChildren() ) {
            if( grandchild instanceof ReferringNodeModel ) {
                conditionText = dereference(((ReferringNodeModel) grandchild).getReference());
            }
        }

        dsb.append(conditionText, " ) {\n");
        if( hasQuality("Priority") ) {
            dsb.addIndent(indentLevel+1);
            dsb.addLine("Priority: ", getQuality("Priority").getValue(), ";");
        }
        for( BaseModel child : children ) {
            if( child instanceof RepeatConditionModel || child instanceof ConditionNodeModel ) {
                continue;
            }
            dsb.addLine(child.decompile(indentLevel+1));
        }
        dsb.sb.deleteCharAt(dsb.sb.length()-1);
        dsb.addBlockCloser(indentLevel);

        indentLevel--;
        dsb.append("\n");
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
