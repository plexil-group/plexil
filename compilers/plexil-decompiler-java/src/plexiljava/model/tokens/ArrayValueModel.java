package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class ArrayValueModel extends NodeModel {

	public ArrayValueModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		
		for( QualityModel quality : qualities ) {
			ret += quality.decompile(0) + " ";
		}
		
		return ret.substring(0, ret.length()-1);
	}
}
