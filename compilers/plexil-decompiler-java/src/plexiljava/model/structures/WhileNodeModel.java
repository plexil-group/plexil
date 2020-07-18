package plexiljava.model.structures;

import plexiljava.main.Constants;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.expressions.ConditionModel;

public class WhileNodeModel extends NodeModel {

	public WhileNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "while( ";
		String condition = getChild("RepeatCondition").decompile(0);
		if( condition.contains(Constants.DECOMPILE_IDENTIFIER_NODEREF) ) {
			String id = condition.substring(Constants.DECOMPILE_IDENTIFIER_NODEREF.length());
			condition = substitute(id);
		}
		ret += condition + " ) {\n";
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			ret += child.decompile(indentLevel+1) + "\n";
		}
		ret = ret.substring(0, ret.length()-1);
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
