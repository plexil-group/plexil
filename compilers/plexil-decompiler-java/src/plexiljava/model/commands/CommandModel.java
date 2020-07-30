package plexiljava.model.commands;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.ArgumentsModel;
import plexiljava.model.tokens.NameModel;

public class CommandModel extends NodeModel {

	public CommandModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( !qualities.isEmpty() ) {
			ret += qualities.get(0).getValue() + " = ";
		}
		ret += getChild(NameModel.class).decompile(0) + "(";
		if( hasChild(ArgumentsModel.class) && !getChild(ArgumentsModel.class).getQualities().isEmpty() ) {
			ret += getChild(ArgumentsModel.class).decompile(0);
		}
		ret += ");";
		return ret;
	}

}
