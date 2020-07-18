package plexiljava.model.expressions;

import plexiljava.model.BaseModel;

public class NOTConditionModel extends ConditionModel {

	public NOTConditionModel(BaseModel node, String type) {
		super(node, type);
	}

	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "!(" + super.decompile(0) + ")";
	}
	
}
