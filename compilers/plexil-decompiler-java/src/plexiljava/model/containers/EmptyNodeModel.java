package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class EmptyNodeModel extends NodeModel {

	public EmptyNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public boolean verify() {
		return hasQuality("NodeId");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("NodeId").getValue(), "{}");
		return dsb.toString();
	}
	
}
