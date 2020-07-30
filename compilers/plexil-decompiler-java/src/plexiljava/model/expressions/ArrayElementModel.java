package plexiljava.model.expressions;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.IndexModel;

public class ArrayElementModel extends NodeModel {

	public ArrayElementModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		return indent(indentLevel) + getQuality("ArrayVariable").getValue() + "[" + getChild(IndexModel.class).decompile(0) + "]";
	}
	
}
