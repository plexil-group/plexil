package plexiljava.model.containers;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.declarations.AssignmentModel;

public class LoopVariableUpdateNodeModel extends NodeModel {

	public LoopVariableUpdateNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return hasChild(AssignmentModel.class);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		return getChild(AssignmentModel.class).decompile(indentLevel);
	}

}
