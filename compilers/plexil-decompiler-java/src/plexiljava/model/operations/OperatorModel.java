package plexiljava.model.operations;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class OperatorModel extends NodeModel {

	protected String operand;
	
	public OperatorModel(BaseModel node, String operand) {
		super(node);
		this.operand = operand;
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel);
		if( children.size() == 2 ) {
			ret = children.get(0).decompile(0) + " " + operand + " " + children.get(1).decompile(0);
		} else if( children.size() == 1 ) {
			if( children.get(0).getOrder() < qualities.get(0).getOrder() ) {
				ret = children.get(0).decompile(0) + " " + operand + " " + qualities.get(0).decompile(0);
			} else {
				ret = qualities.get(0).decompile(0) + " " + operand + " " + children.get(0).decompile(0);
			}
		} else {
			ret = qualities.get(0).decompile(0) + " " + operand + " " + qualities.get(1).decompile(0);
		}
		return ret;
	}

}
