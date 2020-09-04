package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.NOTConditionModel;
import plexiljava.model.conditions.SkipConditionModel;
import plexiljava.model.declarations.DeclareVariableModel;

public class ForNodeModel extends NodeModel {

    public ForNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(DeclareVariableModel.class);
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
        dsb.append("for ( ", getChild(DeclareVariableModel.class).decompile(0), " ");

        BaseModel aux = getChild(AuxNodeModel.class);

        BaseModel skipCondition = aux.getChild(SkipConditionModel.class);
        String condition = "";
        if( skipCondition.hasChild(NOTConditionModel.class) ) {
            condition = skipCondition.getChild(NOTConditionModel.class).getChildren().get(0).decompile(0);
        } else {
            condition = skipCondition.decompile(0); // TODO: Shouldn't this have a NOT in front of it ?
        }

        String update = aux.getChild(LoopVariableUpdateNodeModel.class).decompile(0);

        dsb.append(condition, "; ", update, " ) {\n", aux.decompile(indentLevel+1));
        dsb.addBlockCloser(indentLevel);
        dsb.append("\n");

        indentLevel--;
        dsb.addBlockCloser(indentLevel);
        return dsb.toString();
    }

}
