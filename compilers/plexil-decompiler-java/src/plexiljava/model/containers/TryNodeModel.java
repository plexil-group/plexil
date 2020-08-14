package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;

public class TryNodeModel extends NodeModel {

	public TryNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("Try\n");
		dsb.addIndent(indentLevel);
		dsb.append("{\n");
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			dsb.append(child.decompile(indentLevel+1), "\n");
		}
		dsb.addBlockCloser(indentLevel);
		return dsb.toString();
	}
	
}
