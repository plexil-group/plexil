package plexiljava.model.expressions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.tokens.IndexModel;

public class ArrayElementModel extends NodeModel {

	public ArrayElementModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return hasQuality("ArrayVariable") && hasChild(IndexModel.class);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("ArrayVariable").getValue(), "[", getChild(IndexModel.class).decompile(0), "]");
		return dsb.toString();
	}
	
}
