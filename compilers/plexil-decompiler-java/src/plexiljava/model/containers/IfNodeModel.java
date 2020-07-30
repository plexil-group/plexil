package plexiljava.model.containers;

import plexiljava.main.Constants;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;

public class IfNodeModel extends NodeModel {

	public IfNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "if ( ";
		BaseModel then = getChild("Then");
		String ifCondition = then.getChild("StartCondition").decompile(0);
		if( ifCondition.startsWith(Constants.DECOMPILE_IDENTIFIER_NODEREF) ) {
			String id = ifCondition.substring(Constants.DECOMPILE_IDENTIFIER_NODEREF.length());
			ifCondition = substitute(id);
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
