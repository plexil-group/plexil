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
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		if( hasQuality("NodeRef") ) {
			dsb.addReference(getQuality("NodeRef").getValue());
		} else {
			if( indentLevel != 0 ) {
				dsb.append(getType(), " ");
			}
			if( hasQuality("BooleanValue") ) {
				dsb.append(getQuality("BooleanValue").getValue());
			} else if( hasQuality("BooleanVariable") ) {
				dsb.append(getQuality("BooleanVariable").getValue());
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
