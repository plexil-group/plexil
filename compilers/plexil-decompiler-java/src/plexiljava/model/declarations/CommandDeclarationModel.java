package plexiljava.model.declarations;

import java.util.ArrayList;
import java.util.List;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ParameterModel;

public class CommandDeclarationModel extends NodeModel {

	public CommandDeclarationModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( hasChild("Return") ) {
			ret += getChild("Return").decompile(0) + " ";
		}
		ret += "Command " + getQuality("Name").getValue();
		List<BaseModel> parameters = new ArrayList<BaseModel>();
		for( BaseModel child : children ) {
			if( child instanceof ParameterModel ) {
				parameters.add(child);
			}
		}
		if( !parameters.isEmpty() ) {
			ret += " (";
			for( BaseModel parameter : parameters ) {
				ret += parameter.decompile(0) + ", ";
			}
			ret = ret.substring(0, ret.length()-2);
			ret += ")";
		}
		ret += ";";
		return ret;
	}

}
