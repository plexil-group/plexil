package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class NameModel extends NodeModel {

	public NameModel(BaseModel node) {
		super(node);
	}

	@Override
	public boolean verify() {
		return hasQuality("StringValue");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("StringValue").getValue());
		return dsb.toString();
	}
	
}
