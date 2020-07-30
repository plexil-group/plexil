package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class AssignmentModel extends NodeModel {

	public AssignmentModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 2 ) {
			ret += children.get(0).decompile(0) + " = " + children.get(1).decompile(0);
		} else {
			ret += qualities.get(0).decompile(0) + " = " + children.get(0).decompile(0);
		}
		return ret;
	}

}
