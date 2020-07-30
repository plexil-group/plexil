package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class InitialValueModel extends NodeModel {

	public InitialValueModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( !children.isEmpty() ) {
			ret = children.get(0).decompile(0);
		} else {
			for( QualityModel quality : qualities ) {
				ret += quality.getValue() + " ";
			}
			ret = ret.substring(0, ret.length()-1);
		}
		return ret;
	}
	
}
