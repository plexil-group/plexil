package plexiljava.model.external;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.PairModel;

public class LibraryNodeCallModel extends NodeModel {

    public LibraryNodeCallModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("NodeId");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("LibraryCall ", getQuality("NodeId").getValue(), "(");
        boolean hasAliases = false;
        for( BaseModel child : children ) {
            if( child instanceof PairModel ) {
                dsb.append(child.decompile(0), ", ");
                hasAliases = true;
            }
        }
        if( hasAliases ) {
            dsb.sb.delete(dsb.sb.length()-2, dsb.sb.length());
        }
        dsb.append(");");
        return dsb.toString();
    }

}
