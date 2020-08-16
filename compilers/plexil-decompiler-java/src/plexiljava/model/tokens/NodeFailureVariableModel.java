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
		return hasQuality("NodeId");
	}
	
	@Override
	public String translate(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		dsb.append(getQuality("NodeId").getValue() + ".Failure");
		return dsb.toString();
	}
}
