package plexiljava.model.structures;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ElseIfNodeModel extends NodeModel {

	public ElseIfNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "elseif ( ";
		
		return ret;
	}
	
}
