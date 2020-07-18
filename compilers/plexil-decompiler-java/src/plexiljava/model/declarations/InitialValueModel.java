package plexiljava.model.declarations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class InitialValueModel extends NodeModel {

	public InitialValueModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + qualities.get(0).getValue();
	}
	
}
