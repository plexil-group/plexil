package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class ArgumentsModel extends NodeModel {

    public ArgumentsModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        for( QualityModel quality : qualities ) {
            if( quality.getName().equals("StringValue") && !quality.getValue().startsWith("\"")) {
                dsb.append("\"", quality.getValue(), "\"", ", ");
            } else {
                dsb.append(quality.getValue(), ", ");
            }
        }
        for( BaseModel child : children ) {
            dsb.append(child.decompile(0), ", ");
        }
        dsb.sb.delete(dsb.sb.length()-2, dsb.sb.length());

        return dsb.toString();
    }
}
