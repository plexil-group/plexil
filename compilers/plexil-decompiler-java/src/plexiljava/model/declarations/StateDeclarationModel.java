package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class StateDeclarationModel extends NodeModel {

	public StateDeclarationModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = getChild("Return").decompile(indentLevel) + " Lookup ";
		ret += getQuality("Name").getValue() + ";";
		return ret;
	}
	
}
