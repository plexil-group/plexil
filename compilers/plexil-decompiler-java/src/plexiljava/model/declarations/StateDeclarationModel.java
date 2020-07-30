package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ParameterModel;
import plexiljava.model.tokens.ReturnModel;

public class StateDeclarationModel extends NodeModel {

	public StateDeclarationModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = getChild(ReturnModel.class).decompile(indentLevel) + " Lookup ";
		ret += getQuality("Name").getValue();
		if( hasChild(ParameterModel.class) ) {
			ret += "(" + getChild(ParameterModel.class).decompile(0) + ")";
		}
		ret += ";";
		return ret;
	}
	
}
