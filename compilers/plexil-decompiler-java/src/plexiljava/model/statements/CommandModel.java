package plexiljava.model.statements;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

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
		ret += getChild("Name").getQuality("StringValue").decompile(0) + "(";
		if( hasChild("Arguments") && !getChild("Arguments").getQualities().isEmpty() ) {
			for( QualityModel quality : getChild("Arguments").getQualities() ) {
				if( quality.getName().equals("StringValue") ) {
					ret += "\"" + quality.getValue() + "\"" + ", ";
				} else {
					ret += quality.getValue() + ", ";
				}
			}
			ret = ret.substring(0, ret.length()-2);
		}
		ret += ");";
		return ret;
	}

}
