package plexiljava.model.structures;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.expressions.ConditionModel;

public class ThenNodeModel extends NodeModel {

	public ThenNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = "";
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			ret += child.decompile(indentLevel) + "\n";
		}
		return ret;
	}

}
