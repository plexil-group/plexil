package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class InterfaceModel extends NodeModel {

    public InterfaceModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        for( BaseModel child : children ) {
            if( indentLevel != 0 && children.size() == 1 ) {
                dsb.append(child.decompile(-1), "; ");
            } else {
                dsb.append(child.decompile(0));
                if( indentLevel == 0 ) {
                    dsb.append(", ");
                } else {
                    dsb.append("; ");
                }
            }
        }
        if( !children.isEmpty() ) {
            dsb.sb.delete(dsb.sb.length()-2, dsb.sb.length());
        }
        if( children.size() > 1 && indentLevel != 0 ) {
            dsb.append(";");
        }

        return dsb.toString();
    }

}
