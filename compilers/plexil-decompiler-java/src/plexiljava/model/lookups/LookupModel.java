package plexiljava.model.lookups;

import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;
import plexiljava.model.tokens.NameModel;

public class LookupModel extends TypedNodeModel {

	public LookupModel(BaseModel node) {
		super(node, "Default");
	}
	
	public LookupModel(BaseModel node, String type) {
		super(node, type);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "Lookup (" + getChild(NameModel.class).getQuality("StringValue").getValue() + ")";
	}

}
