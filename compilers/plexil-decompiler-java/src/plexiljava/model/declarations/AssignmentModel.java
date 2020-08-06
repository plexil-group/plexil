package plexiljava.model.declarations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class AssignmentModel extends NodeModel {

	public AssignmentModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return children.size() == 2 || !qualities.isEmpty();
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		if( children.size() == 2 ) {
			dsb.append(children.get(0).decompile(0), " = ", children.get(1).decompile(1));
		} else {
			dsb.append(qualities.get(0).getValue(), " = ", children.get(0).decompile(0));
		}
		return dsb.toString();
	}

}
