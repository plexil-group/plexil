package plexiljava.model;

import plexiljava.model.expressions.ConditionModel;

public class ActionNodeModel extends NodeModel {

	public ActionNodeModel(BaseModel node) {
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
