package plexiljava.model.expressions;

import plexiljava.main.Constants;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ConditionModel extends NodeModel {
	
	protected String type;
	
	public ConditionModel(BaseModel node) {
		this(node, "Generic");
	}
	
	public ConditionModel(BaseModel node, String type) {
		super(node);
		this.type = type;
	}
	
	public String getType() {
		return type;
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 1 ) {
			ret += children.get(0).decompile(0);
		} else if( hasQuality("NodeRef") ) {
			ret += Constants.DECOMPILE_IDENTIFIER_NODEREF + getQuality("NodeRef").decompile(0);
		} else {
			// TODO
		}
		return ret;
	}

}
