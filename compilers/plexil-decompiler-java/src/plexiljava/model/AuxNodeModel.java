package plexiljava.model;

import plexiljava.model.conditions.ConditionModel;

public class AuxNodeModel extends NodeModel {

	public AuxNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = "";
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel || child.hasAttribute("epx") && child.getAttribute("epx").getValue().equals("LoopVariableUpdate") ) {
				continue;
			}
			ret += child.decompile(indentLevel) + "\n";
		}
		return ret;
	}
	
}
