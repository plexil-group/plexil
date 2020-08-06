package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.declarations.AssignmentModel;

public class AssignmentNodeModel extends NodeModel {

	public AssignmentNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return hasQuality("NodeId") && (children.size() != 1 || hasChild(AssignmentModel.class));
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		if( indentLevel == 0 ) {
			dsb.append(getChild(AssignmentModel.class).decompile(0), ";");
		} else if( children.size() == 1 ) {
			dsb.append(getQuality("NodeId").getValue(), ": ", getChild(AssignmentModel.class).decompile(0), ";");
		} else {
			dsb.append(getQuality("NodeId").getValue());
			dsb.addBlockOpener();
			for( BaseModel child : children ) {
				dsb.append(child.decompile(indentLevel+1));
				if( child instanceof AssignmentModel ) {
					dsb.append(";");
				}
				dsb.append("\n");
			}
			dsb.addBlockCloser(indentLevel);
		}
		return dsb.toString();
	}

}
