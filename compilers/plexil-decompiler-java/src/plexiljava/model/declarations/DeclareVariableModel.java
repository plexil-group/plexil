package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class DeclareVariableModel extends NodeModel {

	public DeclareVariableModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + getQuality("Type").decompile(0) + " " + getQuality("Name").decompile(0);
		if( children.size() > 0 ) {
			ret += " = ";
			ret += getChild("InitialValue").getQualities().get(0).decompile(0);
		}
		ret += ";";
		return ret;
	}
	
}
