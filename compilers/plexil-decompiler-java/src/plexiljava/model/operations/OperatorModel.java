package plexiljava.model.operations;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;

public class OperatorModel extends TypedNodeModel {
	
	public OperatorModel(BaseModel node) {
		super(node, "Default");
	}
	
	protected OperatorModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);

		if( children.size() == 2 ) {
			dsb.append(children.get(0).decompile(0), " ", type, " ", children.get(1).decompile(0));
		} else if( children.size() == 1 ) {
			if( children.get(0).getOrder() < qualities.get(0).getOrder() ) {
				dsb.append(children.get(0).decompile(0), " ", type, " ", qualities.get(0).getValue());
			} else {
				dsb.append(qualities.get(0).getValue(), " ", type, " ", children.get(0).decompile(0));
			}
		} else {
			dsb.append(qualities.get(0).getValue(), " ", type, " ", qualities.get(1).getValue());
		}
		
		return dsb.toString();
	}

}
