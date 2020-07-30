package plexiljava.model.containers;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;

public class ConcurrenceNodeModel extends NodeModel {

	public ConcurrenceNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + getQuality("NodeId").decompile(0) + ": Concurrence {\n";
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				switch( ((ConditionModel) child).getType() ) {
					case "Start":
						ret += child.decompile(indentLevel+1) + "\n";
						break;
					case "End":
						ret += child.decompile(indentLevel+1) + "\n";
						break;
					case "Repeat":
						ret += child.decompile(indentLevel+1) + "\n";
						break;
					default:
						continue;
				}
			} else {
				ret += child.decompile(indentLevel+1) + "\n";
			}
		}
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
