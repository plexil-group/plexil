package plexiljava.model.expressions;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.lookups.LookupModel;
import plexiljava.model.operations.OperatorModel;

public class BooleanRHSModel extends NodeModel {
	
	public BooleanRHSModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( hasChild(OperatorModel.class) ) {
			ret += getChild(OperatorModel.class).decompile(0);
		} else if( hasChild(LookupModel.class) ) {
			ret += getChild(LookupModel.class).decompile(0);
		}
		else {
			ret += qualities.get(0).decompile(0);
		}
		return ret;
	}
}
