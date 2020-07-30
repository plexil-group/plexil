package plexiljava.model.tokens;

import plexiljava.model.BaseModel;

public class AnyParametersModel extends ParameterModel {

	public AnyParametersModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "...";
	}
	
}
