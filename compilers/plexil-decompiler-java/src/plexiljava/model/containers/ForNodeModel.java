package plexiljava.model.containers;

import plexiljava.model.AuxNodeModel;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.NOTConditionModel;
import plexiljava.model.declarations.DeclareVariableModel;

public class ForNodeModel extends NodeModel {

	public ForNodeModel(BaseModel node) {
		super(node);
	}

	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + "for( " + getChild(DeclareVariableModel.class).decompile(0) + " ";
		BaseModel aux = null;
		for( BaseModel child : children ) {
			if( child instanceof AuxNodeModel ) {
				aux = child;
				break;
			}
		}
		String condition = "";
		for( BaseModel grandchild : aux.getChildren() ) {
			if( grandchild instanceof ConditionModel ) {
				if( ((ConditionModel) grandchild).getType().equals("Skip") ) {
					if( grandchild.hasChild(NOTConditionModel.class) ) {
						condition = grandchild.getChild(NOTConditionModel.class).getChildren().get(0).decompile(0);
					} else {
						condition = grandchild.decompile(0);
					}
				}
			}
		}
		String update = "";
		for( BaseModel grandchild : aux.getChildren() ) {
			if( grandchild.hasAttribute("epx") && grandchild.getAttribute("epx").getValue().equals("LoopVariableUpdate") ) {
				update = grandchild.decompile(0);
			}
		}
		ret += condition + "; " + update + " ) {\n";
		ret += aux.decompile(indentLevel+1);
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
