package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NameModel extends NodeModel {

	public NameModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + getQuality("StringValue").getValue();
	}
	
}
