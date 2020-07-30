package plexiljava.model.containers;

import plexiljava.main.Constants;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.StartConditionModel;

public class IfNodeModel extends NodeModel {

	public IfNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "if ( ";
		BaseModel then = getChild(ThenNodeModel.class);
		String ifCondition = then.getChild(StartConditionModel.class).decompile(0);
		if( ifCondition.startsWith(Constants.DECOMPILE_IDENTIFIER_NODEREF) ) {
			String id = ifCondition.substring(Constants.DECOMPILE_IDENTIFIER_NODEREF.length());
			ifCondition = dereference(id);
		}
		ret += ifCondition + " ) {\n";
		for( BaseModel child : then.getChildren() ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			ret += child.decompile(indentLevel+1) + "\n";
		}
		return ret;
	}
	
}
