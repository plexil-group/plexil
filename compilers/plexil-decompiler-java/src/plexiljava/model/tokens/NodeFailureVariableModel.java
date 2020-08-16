package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NodeFailureVariableModel extends NodeModel {

	public NodeFailureVariableModel(BaseModel node) {
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
		
		dsb.append(getQuality("NodeRef").getValue() + ".Failure");
		return dsb.toString();
	}
}
