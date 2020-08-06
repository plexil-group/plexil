package plexiljava.model.lookups;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;
import plexiljava.model.tokens.ArgumentsModel;
import plexiljava.model.tokens.NameModel;

public class LookupModel extends TypedNodeModel {

	public LookupModel(BaseModel node) {
		super(node, "Default");
	}
	
	public LookupModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public boolean verify() {
		return hasChild(NameModel.class);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		dsb.append("Lookup (", getChild(NameModel.class).getQuality("StringValue").getValue());
		if( hasChild(ArgumentsModel.class) ) {
			dsb.append("(", getChild(ArgumentsModel.class).decompile(0), ")");
		}
		dsb.append(")");
		
		return dsb.toString();
	}

}
