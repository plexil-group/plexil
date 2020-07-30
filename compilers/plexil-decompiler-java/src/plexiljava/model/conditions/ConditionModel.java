package plexiljava.model.conditions;

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
		String ret = indent(indentLevel);
		if( hasQuality("NodeRef") ) {
			ret += "@" + getQuality("NodeRef").decompile(0);
		} else {
			if( indentLevel != 0 ) {
				ret += getType() + " ";
			}
			if( hasQuality("BooleanValue") ) {
				ret += getQuality("BooleanValue").getValue();
			} else if( hasQuality("BooleanVariable") ) {
				ret += getQuality("BooleanVariable").getValue();
			} else if( !children.isEmpty() ){
				ret += children.get(0).decompile(0);
			}
			if( indentLevel != 0 ) {
				ret += ";";
			}
		}
		return ret;
	}

}
