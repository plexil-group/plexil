package plexiljava.model.declarations;

import java.util.ArrayList;
import java.util.List;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ParameterModel;
import plexiljava.model.tokens.ReturnModel;

public class CommandDeclarationModel extends NodeModel {

    public CommandDeclarationModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("Name");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( hasChild(ReturnModel.class) ) {
            dsb.append(getChild(ReturnModel.class).decompile(0), " ");
        }
        dsb.append("Command ", getQuality("Name").getValue());

        List<BaseModel> parameters = new ArrayList<BaseModel>();
        for( BaseModel child : children ) {
            if( child instanceof ParameterModel ) {
                parameters.add(child);
            }
        }

        if( !parameters.isEmpty() ) {
            dsb.append(" (");
            for( BaseModel parameter : parameters ) {
                dsb.append(parameter.decompile(0), ", ");
            }
            dsb.sb.delete(dsb.sb.length()-2, dsb.sb.length());
            dsb.append(")");
        }
        dsb.append(";");
        return dsb.toString();
    }

}
