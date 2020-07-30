package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

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
			ret += "(" + getChild(InitialValueModel.class).decompile(0) + ")";
		}
		ret += ";";
		return ret;
	}

}
