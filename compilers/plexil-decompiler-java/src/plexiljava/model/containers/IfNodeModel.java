package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.StartConditionModel;

public class IfNodeModel extends NodeModel {

	public IfNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public boolean verify() {
		return hasChild(ThenNodeModel.class);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("if ( ");
		
		BaseModel then = getChild(ThenNodeModel.class);
		String ifCondition = then.getChild(StartConditionModel.class).decompile(0);
		if( ifCondition.startsWith("@") ) {
			String id = ifCondition.substring(1);
			ifCondition = dereference(id);
		}
		
		dsb.append(ifCondition, " ) {\n");
		
		for( BaseModel child : then.getChildren() ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			dsb.addLine(child.decompile(indentLevel+1));
		}
		
		return dsb.toString();
	}
	
}
