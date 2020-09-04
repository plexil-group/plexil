package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.InterfaceModel;

public class LibraryNodeDeclarationModel extends NodeModel {

    public LibraryNodeDeclarationModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        dsb.append("LibraryAction ", getQuality("Name").getValue(), " ", "(");
        if( hasChild(InterfaceModel.class) ) {
            dsb.append(getChild(InterfaceModel.class).decompile(0));
        }
        dsb.append(");");

        return dsb.toString();
    }

}
