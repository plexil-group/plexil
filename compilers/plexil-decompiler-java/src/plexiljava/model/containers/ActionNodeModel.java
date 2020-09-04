package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.BaseModel.PatternRecognitionFailureException;
import plexiljava.model.conditions.ConditionModel;

public class ActionNodeModel extends NodeModel {

    public ActionNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();

        for( BaseModel child : children ) {
            if( child instanceof ConditionModel ) {
                continue;
            }

            dsb.addLine(child.decompile(indentLevel));
        }

        return dsb.toString();
    }

}
