package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class ArrayValueModel extends NodeModel {

    public ArrayValueModel(BaseModel node) {
        super(node);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        for( QualityModel quality : qualities ) {
            dsb.append(quality.getValue(), " ");
        }
        dsb.sb.deleteCharAt(dsb.sb.length()-1);

        return dsb.toString();
    }
}
