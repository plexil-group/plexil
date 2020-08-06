package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ElseIfNodeModel extends NodeModel {

	public ElseIfNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("elseif (");		
		return dsb.toString();
	}
	
}
