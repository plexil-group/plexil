package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class IsKnownModel extends NodeModel {

	public IsKnownModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return !qualities.isEmpty();
	}
	
	@Override
	public String translate(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("isKnown(", qualities.get(0).getValue(), ")");
		return dsb.toString();
	}

}
