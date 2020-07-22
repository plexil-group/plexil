package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ReturnModel extends NodeModel {

	public ReturnModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return getQuality("Type").decompile(indentLevel);
	}
	
}
