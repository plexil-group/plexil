package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;

public class AnyParametersModel extends ParameterModel {

	public AnyParametersModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("...");
		return dsb.toString();
	}
	
}
