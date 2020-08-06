package plexiljava.model.states;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class StateModel extends TypedNodeModel {

	public StateModel(BaseModel node) {
		super(node);
	}

	public StateModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public boolean verify() {
		return hasQuality("NodeRef");
	}
	
	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("NodeRef").getValue(), ".status == ", type);
		return dsb.toString();
	}
	
}