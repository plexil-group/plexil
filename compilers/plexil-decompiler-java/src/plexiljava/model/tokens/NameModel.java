package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.operators.OperatorModel;

public class NameModel extends NodeModel {

    public NameModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasQuality("StringValue") || hasQuality("StringVariable") || hasChild(OperatorModel.class);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( hasQuality("StringValue") ) {
            dsb.append(getQuality("StringValue").getValue());
        } else if( hasQuality("StringVariable") ) {
            dsb.append(getQuality("StringVariable").getValue());
        } else {
            dsb.append(children.get(0).decompile(0));
        }
        return dsb.toString();
    }

}
