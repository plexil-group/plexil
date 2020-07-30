package plexiljava.model.operations;

import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class OperatorModel extends TypedNodeModel {
	
	public OperatorModel(BaseModel node) {
		super(node, "Default");
	}
	
	protected OperatorModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 2 ) {
			ret = children.get(0).decompile(0) + " " + type + " " + children.get(1).decompile(0);
		} else if( children.size() == 1 ) {
			if( children.get(0).getOrder() < qualities.get(0).getOrder() ) {
				ret = children.get(0).decompile(0) + " " + type + " " + qualities.get(0).decompile(0);
			} else {
				ret = qualities.get(0).decompile(0) + " " + type + " " + children.get(0).decompile(0);
			}
		} else {
			ret = qualities.get(0).decompile(0) + " " + type + " " + qualities.get(1).decompile(0);
		}
		return ret;
	}

}
