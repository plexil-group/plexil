package plexiljava.model.conditions;

import plexiljava.model.BaseModel;

public class ConditionNodeModel extends ConditionModel {

	public ConditionNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return children.get(0).decompile(indentLevel);
	}
}
