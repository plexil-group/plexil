package plexiljava.model.commands;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ArgumentsModel;
import plexiljava.model.tokens.NameModel;

public class CommandModel extends NodeModel {

    public CommandModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(NameModel.class);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( !qualities.isEmpty() ) {
            dsb.append(qualities.get(0).getValue(), " = ");
        }
        dsb.append(getChild(NameModel.class).decompile(0), "(");
        if( hasChild(ArgumentsModel.class) && (!getChild(ArgumentsModel.class).getQualities().isEmpty() || !getChild(ArgumentsModel.class).getChildren().isEmpty()) ) {
            dsb.append(getChild(ArgumentsModel.class).decompile(0));
        }
        dsb.append(");");
        return dsb.toString();
    }

}
