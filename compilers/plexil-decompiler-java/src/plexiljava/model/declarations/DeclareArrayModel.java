package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class DeclareArrayModel extends NodeModel {

    public DeclareArrayModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("Type") && hasQuality("Name") && hasQuality("MaxSize") && (children.isEmpty() || hasChild(InitialValueModel.class));
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(getQuality("Type").getValue(), " ", getQuality("Name").getValue(), "[", getQuality("MaxSize").getValue(), "]");

        if( children.size() > 0 ) {
            dsb.append(" = #");
            dsb.append("(", getChild(InitialValueModel.class).decompile(0), ")");
        }
        dsb.append(";");

        return dsb.toString();
    }

}
