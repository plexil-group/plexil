package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ParameterModel extends NodeModel {

	public ParameterModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		return getQuality("Type").decompile(indentLevel);
	}

}
