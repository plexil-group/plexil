package plexiljava.model.external;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.PairModel;

public class UpdateModel extends NodeModel {

    public UpdateModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !children.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("Update ");
        boolean first = true;
        for( BaseModel child : children ) {
            if( child instanceof PairModel ) {
                if( first ) {
                    first = false;
                } else {
                    dsb.append(", ");
                }
                dsb.append(child.decompile(0));
            }
        }
        dsb.append(";");
        return dsb.toString();
    }

}
