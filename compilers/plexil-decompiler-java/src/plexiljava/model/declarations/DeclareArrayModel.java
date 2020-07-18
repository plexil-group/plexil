package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.QualityModel;

public class DeclareArrayModel extends NodeModel {

	public DeclareArrayModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + getQuality("Type").decompile(0) + " " + getQuality("Name").decompile(0) + "[" + getQuality("MaxSize").decompile(0) + "]";
		if( children.size() > 0 ) {
			ret += " = ";
			switch( getQuality("Type").getValue() ) {
				case "Real":
					ret += "#";
					break;
				default:
					break;
			}
			ret += "(";
			BaseModel container = getChild("InitialValue");
			if( container.getChildren().size() > 0 ) {
				container = container.getChild("ArrayValue");
			}
			for( QualityModel quality : container.getQualities() ) {
				ret += quality.decompile(0) + " ";
			}
			ret = ret.substring(0, ret.length()-1);
			ret += ")";
		}
		ret += ";";
		return ret;
	}

}
