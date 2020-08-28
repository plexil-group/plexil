package plexiljava.model;

import plexiljava.decompilation.DecompilableStringBuilder;

public class ReferringNodeModel extends NodeModel {

	public ReferringNodeModel(BaseModel node) {
		super(node);
	}

	public String getReference() {
		return hasQuality("NodeRef") ? getQuality("NodeRef").getValue() : getQuality("NodeId").getValue();
	}
	
	@Override
	public boolean verify() {
		return hasQuality("NodeRef") || hasQuality("NodeId");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.addReference(getReference());
		return dsb.toString();
	}
}
