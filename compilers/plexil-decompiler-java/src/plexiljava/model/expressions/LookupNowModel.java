package plexiljava.model.expressions;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class LookupNowModel extends NodeModel {

	public LookupNowModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + "Lookup (" + getChild("Name").getQuality("StringValue").getValue() + ")";
	}
}
