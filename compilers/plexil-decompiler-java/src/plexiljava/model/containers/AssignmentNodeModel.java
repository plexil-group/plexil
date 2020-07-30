package plexiljava.model.containers;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.declarations.AssignmentModel;

public class AssignmentNodeModel extends NodeModel {

	public AssignmentNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 1 ) {
			if( indentLevel != 0 ) {
				ret += getQuality("NodeId").getValue() + ": ";
			}
			ret += getChild(AssignmentModel.class).decompile(0) + ";";
		} else {
			ret += getQuality("NodeId").getValue() + ": {\n";
			for( BaseModel child : children ) {
				ret += child.decompile(indentLevel+1);
				if( child instanceof AssignmentModel ) {
					ret += ";";
				}
				ret += "\n";
			}
			ret += indent(indentLevel) + "}";
		}
		return ret;
	}

}
