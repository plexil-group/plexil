package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class IndexModel extends NodeModel {

	public IndexModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + qualities.get(0).getValue();
	}

}
