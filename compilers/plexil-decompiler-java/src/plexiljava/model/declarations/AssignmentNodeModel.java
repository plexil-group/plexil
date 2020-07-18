package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class AssignmentNodeModel extends NodeModel {

	public AssignmentNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( indentLevel != 0 ) {
			ret += getQuality("NodeId").getValue() + ": ";
		}
		ret += getChild("Assignment").decompile(0);
		return ret;
	}

}
