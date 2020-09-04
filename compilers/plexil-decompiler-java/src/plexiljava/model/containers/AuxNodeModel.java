package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.SkipConditionModel;

public class AuxNodeModel extends NodeModel {

    public AuxNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(SkipConditionModel.class) && hasChild(LoopVariableUpdateNodeModel.class);
    }
    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();

        for( BaseModel child : children ) {
            if( child instanceof ConditionModel || child.hasAttribute("epx") && child.getAttribute("epx").getValue().equals("LoopVariableUpdate") ) {
                continue;
            }

            dsb.addLine(child.decompile(indentLevel));
        }

        return dsb.toString();
    }

}
