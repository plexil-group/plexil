package plexiljava.model.containers;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class EmptyNodeModel extends NodeModel {

	public EmptyNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + getQuality("NodeId").getValue() + "{}";
		return ret;
	}
	
}
