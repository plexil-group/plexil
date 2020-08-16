package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class AliasModel extends NodeModel {

	public AliasModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return qualities.size() >= 2;
	}
	
	@Override
	public String translate(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		dsb.append(qualities.get(0).getValue(), " = ", qualities.get(1).getValue());
		
		return dsb.toString();
	}
	

}
