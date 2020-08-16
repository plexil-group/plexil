package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NodeOutcomeVariableModel extends NodeModel {

	public NodeOutcomeVariableModel(BaseModel node) {
		super(node);
	}

	@Override
	public boolean verify() {
		return hasQuality("NodeRef");
	}
	
	@Override
	public String translate(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		dsb.append(getQuality("NodeRef").getValue() + ".outcome");
		return dsb.toString();
	}
}
