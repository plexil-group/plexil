package plexiljava.model.commands;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class CommandNodeModel extends NodeModel {

	public CommandNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = "";
		ret += indent(indentLevel) + getQuality("NodeId").getValue() + ": {\n";
		for( BaseModel child : children ) {
			ret += child.decompile(indentLevel+1) + "\n";
		}
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
