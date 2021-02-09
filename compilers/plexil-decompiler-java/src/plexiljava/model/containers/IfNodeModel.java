package plexiljava.model.containers;

import java.util.ArrayList;
import java.util.List;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.ConditionNodeModel;
import plexiljava.model.conditions.StartConditionModel;

public class IfNodeModel extends NodeModel {

    public IfNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(ThenNodeModel.class);
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

        List<BaseModel> elseifs = new ArrayList<BaseModel>();
        List<BaseModel> thens = new ArrayList<BaseModel>();
        for( BaseModel child : children ) {
            if( child instanceof ConditionModel && !(child instanceof ConditionNodeModel) ) {
                dsb.addLine(child.decompile(indentLevel));
            } else if( child instanceof ElseIfNodeModel ) {
                elseifs.add(child);
            } else if( child instanceof ThenNodeModel ) {
                thens.add(child);
            }
        }

        dsb.addIndent(indentLevel);
        dsb.append("if ( ");

        BaseModel then = thens.remove(0);
        String ifCondition;
        if( hasChild(ConditionNodeModel.class) ) {
            ifCondition = getChild(ConditionNodeModel.class).decompile(0);
        } else {
            ifCondition = then.getChild(StartConditionModel.class).decompile(0);
            if( ifCondition.startsWith(DecompilableStringBuilder.REFERENCE_IDENTIFIER) ) {
                String id = ifCondition.substring(1);
                ifCondition = dereference(id);
            }
        }
        dsb.append(ifCondition, " ) {\n");

        for( BaseModel child : then.getChildren() ) {
            if( child instanceof ConditionModel ) {
                continue;
            }
            dsb.addLine(child.decompile(indentLevel+1));
        }
        dsb.addBlockCloser(indentLevel);
        dsb.append("\n");

        for( BaseModel elseif : elseifs ) {
            dsb.append(elseif.decompile(indentLevel));

            then = thens.remove(0);
            for( BaseModel child : then.getChildren() ) {
                if( child instanceof ConditionModel ) {
                    continue;
                }
                dsb.addLine(child.decompile(indentLevel+1));
            }
            dsb.addBlockCloser(indentLevel);
            dsb.append("\n");
        }

        if( hasChild(ElseNodeModel.class) ) {
            dsb.append(getChild(ElseNodeModel.class).decompile(indentLevel));
            for( BaseModel child : getChild(ElseNodeModel.class).getChildren() ) {
                if( child instanceof ConditionModel ) {
                    continue;
                }
                dsb.addLine(child.decompile(indentLevel+1));
            }
            dsb.addBlockCloser(indentLevel);
            dsb.append("\n");
        }

        indentLevel--;
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
