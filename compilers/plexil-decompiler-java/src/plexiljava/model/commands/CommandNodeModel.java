package plexiljava.model.commands;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class CommandNodeModel extends NodeModel {

	public CommandNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return getChild("Command").decompile(indentLevel);
	}
	
}
