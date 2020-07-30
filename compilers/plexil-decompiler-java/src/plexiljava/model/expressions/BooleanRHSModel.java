package plexiljava.model.expressions;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class BooleanRHSModel extends NodeModel {
	
	public BooleanRHSModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 1 ) {
			ret += children.get(0).decompile(0);
		} else {
			ret += qualities.get(0).decompile(0);
		}
		return ret;
	}
}
