package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class ArgumentsModel extends NodeModel {

	public ArgumentsModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		for( QualityModel quality : qualities ) {
			if( quality.getName().equals("StringValue") ) {
				ret += "\"" + quality.getValue() + "\"" + ", ";
			} else {
				ret += quality.getValue() + ", ";
			}
		}
		return ret.substring(0, ret.length()-2);
	}
}
