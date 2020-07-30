package plexiljava.model.states;

import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class StateModel extends TypedNodeModel {

	public StateModel(BaseModel node) {
		super(node);
	}

	public StateModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + getQuality("NodeRef").getValue() + ".status == " + type;
	}
	
}