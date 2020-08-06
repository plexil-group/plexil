package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;

public class ConcurrenceNodeModel extends NodeModel {

	public ConcurrenceNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public boolean verify() {
		return hasQuality("NodeId");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append(getQuality("NodeId").getValue());
		dsb.addBlockOpener("Concurrence");
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				switch( ((ConditionModel) child).getType() ) {
					case "Start":
						dsb.addLine(child.decompile(indentLevel+1));
						break;
					case "End":
						dsb.addLine(child.decompile(indentLevel+1));
						break;
					case "Repeat":
						dsb.addLine(child.decompile(indentLevel+1));
						break;
					default:
						continue;
				}
			} else {
				dsb.addLine(child.decompile(indentLevel+1));
			}
		}
		dsb.addBlockCloser(indentLevel);
		return dsb.toString();
	}
	
}
