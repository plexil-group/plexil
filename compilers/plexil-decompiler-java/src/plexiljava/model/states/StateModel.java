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
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("NodeRef").getValue(), ".state == ", type);
		return dsb.toString();
	}
	
}