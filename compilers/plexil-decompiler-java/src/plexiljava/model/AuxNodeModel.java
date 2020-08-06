package plexiljava.model;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.conditions.ConditionModel;

public class AuxNodeModel extends NodeModel {

	public AuxNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel || child.hasAttribute("epx") && child.getAttribute("epx").getValue().equals("LoopVariableUpdate") ) {
				continue;
			}
			
			dsb.addLine(child.decompile(indentLevel));
		}
		
		return dsb.toString();
	}
	
}
