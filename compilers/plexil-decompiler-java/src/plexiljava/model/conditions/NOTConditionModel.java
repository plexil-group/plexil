package plexiljava.model.conditions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;

public class NOTConditionModel extends ConditionModel {

	public NOTConditionModel(BaseModel node) {
		super(node, "");
	}

	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("!(", super.decompile(0), ")");
		return dsb.toString();
	}
	
}
