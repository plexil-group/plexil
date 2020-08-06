package plexiljava.model;

import plexiljava.decompilation.DecompilableStringBuilder;

public class ReferringNodeModel extends NodeModel {

	public ReferringNodeModel(BaseModel node) {
		super(node);
	}

	public String getReference() {
		return getQuality("NodeRef").getValue();
	}
	
	@Override
	public boolean verify() {
		return hasQuality("NodeRef");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.addReference(getReference());
		return dsb.toString();
	}
}
