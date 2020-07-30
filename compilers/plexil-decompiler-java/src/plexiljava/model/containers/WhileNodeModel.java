package plexiljava.model.containers;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.ReferringNodeModel;
import plexiljava.model.conditions.RepeatConditionModel;

public class WhileNodeModel extends NodeModel {

	public WhileNodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "while( ";
		
		RepeatConditionModel repeatCondition = (RepeatConditionModel) getChild(RepeatConditionModel.class);
		String conditionText = repeatCondition.decompile(0);
		for( BaseModel grandchild : repeatCondition.getChildren() ) {
			if( grandchild instanceof ReferringNodeModel ) {
				conditionText = dereference(((ReferringNodeModel) grandchild).getReference());
			}
		}
		
		ret += conditionText + " ) {\n";
		for( BaseModel child : children ) {
			if( child instanceof RepeatConditionModel ) {
				continue;
			}
			ret += child.decompile(indentLevel+1) + "\n";
		}
		ret = ret.substring(0, ret.length()-1);
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
