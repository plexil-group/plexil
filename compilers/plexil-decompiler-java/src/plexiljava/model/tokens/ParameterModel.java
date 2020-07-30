package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ParameterModel extends NodeModel {

	public ParameterModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return getQuality("Type").decompile(indentLevel);
	}

}
