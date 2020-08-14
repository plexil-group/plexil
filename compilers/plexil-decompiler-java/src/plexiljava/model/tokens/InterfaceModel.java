package plexiljava.model.tokens;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class InterfaceModel extends NodeModel {

	public InterfaceModel(BaseModel node) {
		super(node);
	}

	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		for( BaseModel child : children ) {
			dsb.append(child.decompile(0), ", ");
		}
		if( !children.isEmpty() ) {
			dsb.sb.delete(dsb.sb.length()-2, dsb.sb.length());
		}
		
		return dsb.toString();
	}
	
}
