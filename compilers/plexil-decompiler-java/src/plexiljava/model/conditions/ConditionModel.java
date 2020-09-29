package plexiljava.model.conditions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class ConditionModel extends TypedNodeModel {

    public ConditionModel(BaseModel node) {
        this(node, "Generic");
    }

    protected ConditionModel(BaseModel node, String type) {
        super(node, type);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        if( hasQuality("NodeRef") ) {
            dsb.addReference(getQuality("NodeRef").getValue());
        } else {
            if( indentLevel != 0 ) {
                dsb.append(getType(), " ");
            }
            if( hasQuality("BooleanValue") ) {
                String bool = getQuality("BooleanValue").getValue();
                if( bool.equals("0") ) {
                    bool = "false";
                }
                if( bool.equals("1") ) {
                    bool = "true";
                }
                dsb.append(bool);
            } else if( hasQuality("BooleanVariable") ) {
                String bool = getQuality("BooleanVariable").getValue();
                if( bool.equals("0") ) {
                    bool = "false";
                }
                if( bool.equals("1") ) {
                    bool = "true";
                }
                dsb.append(bool);
            } else if( !children.isEmpty() ){
                dsb.append(children.get(0).decompile(0));
            }
            if( indentLevel != 0 ) {
                dsb.append(";");
            }
        }
        return dsb.toString();
    }

}
