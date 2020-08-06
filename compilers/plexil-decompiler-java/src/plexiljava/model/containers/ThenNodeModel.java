package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.StartConditionModel;

public class ThenNodeModel extends NodeModel {

	public ThenNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return hasChild(StartConditionModel.class);
	}
	
	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			dsb.addLine(child.decompile(indentLevel));
		}
		return dsb.toString();
	}

}
