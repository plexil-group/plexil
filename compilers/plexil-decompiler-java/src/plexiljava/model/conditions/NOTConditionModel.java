package plexiljava.model.conditions;

import plexiljava.model.BaseModel;

public class NOTConditionModel extends ConditionModel {

	public NOTConditionModel(BaseModel node) {
		super(node, "");
	}

	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "!(" + super.decompile(0) + ")";
	}
	
}
