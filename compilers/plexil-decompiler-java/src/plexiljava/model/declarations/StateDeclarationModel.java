package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ParameterModel;
import plexiljava.model.tokens.ReturnModel;

public class StateDeclarationModel extends NodeModel {

    public StateDeclarationModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(ReturnModel.class) && hasQuality("Name");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.append(getChild(ReturnModel.class).decompile(indentLevel), " Lookup ", getQuality("Name").getValue());

        if( hasChild(ParameterModel.class) ) {
            dsb.append("(", getChild(ParameterModel.class).decompile(0), ")");
        }
        dsb.append(";");

        return dsb.toString();
    }

}
