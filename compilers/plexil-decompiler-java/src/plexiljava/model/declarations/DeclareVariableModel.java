package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class DeclareVariableModel extends NodeModel {

    public DeclareVariableModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("Type") && hasQuality("Name") && (children.isEmpty() || hasChild(InitialValueModel.class));
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append(getQuality("Type").getValue(), " ", getQuality("Name").getValue());
        if( children.size() > 0 ) {
            dsb.append(" = ");
            if( getQuality("Type").getValue().equals("String") && !getChild(InitialValueModel.class).decompile(0).startsWith("\"") ) {
                dsb.append("\"");
            }
            dsb.append(getChild(InitialValueModel.class).decompile(0));
            if( getQuality("Type").getValue().equals("String") && !getChild(InitialValueModel.class).decompile(0).endsWith("\"") ) {
                dsb.append("\"");
            }
        }
        dsb.append(";");
        return dsb.toString();
    }

}
