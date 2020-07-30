package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ReturnModel;

public class StateDeclarationModel extends NodeModel {

	public StateDeclarationModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = getChild(ReturnModel.class).decompile(indentLevel) + " Lookup ";
		ret += getQuality("Name").getValue() + ";";
		return ret;
	}
	
}
