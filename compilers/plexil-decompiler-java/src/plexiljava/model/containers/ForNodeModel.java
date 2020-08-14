package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
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
	public boolean verify() {
		return hasChild(DeclareVariableModel.class);
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		dsb.append("for ( ", getChild(DeclareVariableModel.class).decompile(0), " ");

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
		
		dsb.append(condition, "; ", update.substring(0, update.length()-1), " ) {\n", aux.decompile(indentLevel+1));
		dsb.addBlockCloser(indentLevel);
		return dsb.toString();
	}
	
}
