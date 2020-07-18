package plexiljava.model;

import plexiljava.decompilation.Decompilable;
import plexiljava.model.declarations.AssignmentModel;
import plexiljava.model.declarations.AssignmentNodeModel;
import plexiljava.model.declarations.DeclareArrayModel;
import plexiljava.model.declarations.DeclareVariableModel;
import plexiljava.model.declarations.InitialValueModel;
import plexiljava.model.expressions.ArrayElementModel;
import plexiljava.model.expressions.ConditionModel;
import plexiljava.model.expressions.NOTConditionModel;
import plexiljava.model.expressions.NumericRHSModel;
import plexiljava.model.expressions.OperatorModel;
import plexiljava.model.structures.ForNodeModel;
import plexiljava.model.structures.WhileNodeModel;

public class NodeModel extends BaseModel implements Decompilable {
		
	public NodeModel(BaseModel node) {
		super(node);
	}
	
	@Override
	protected void addBranches() {
		for( QualityModel quality : root.getQualities() ) {
			qualities.add(quality);
		}
		
		for( BaseModel child : root.getChildren() ) {
			generateChild(child);
		}
	}
	
	public void generateChild(BaseModel child) {
		// TODO: Consolidate categories, like skip
		switch( child.getName() ) {
			case "VariableDeclarations":
				for( BaseModel grandchild : child.getChildren() ) {
					generateChild(grandchild);
				}
				break;
			case "DeclareArray":
				children.add(new DeclareArrayModel(child));
				break;
			case "DeclareVariable":
				children.add(new DeclareVariableModel(child));
				break;
			case "InitialValue":
				children.add(new InitialValueModel(child));
				break;
			case "InvariantCondition":
				children.add(new ConditionModel(child, "Invariant"));
				break;
			case "StartCondition":
				children.add(new ConditionModel(child, "Start"));
				break;
			case "SkipCondition":
				children.add(new ConditionModel(child, "Skip"));
				break;
			case "PostCondition":
				generateChild(child.getChildren().get(0));
				break;
			case "NodeBody":
				generateChild(child.getChildren().get(0));
				break;
			case "NodeList":
				for( BaseModel grandchild : child.getChildren() ) {
					generateChild(grandchild);
				}
				break;
			case "Assignment":
				children.add(new AssignmentModel(child));
				break;
			case "ArrayElement":
				children.add(new ArrayElementModel(child));
				break;
			case "NumericRHS":
				children.add(new NumericRHSModel(child));
				break;
			case "ADD":
				children.add(new OperatorModel(child, "+"));
				break;
			case "SUB":
				children.add(new OperatorModel(child, "-"));
				break;
			case "MUL":
				children.add(new OperatorModel(child, "*"));
				break;
			case "DIV":
				children.add(new OperatorModel(child, "/"));
				break;
			case "LT":
				children.add(new OperatorModel(child, "<"));
				break;
			case "GT":
				children.add(new OperatorModel(child, ">"));
				break;
			case "EQ":
				children.add(new OperatorModel(child, "=="));
				break;
			case "NEQ":
				children.add(new OperatorModel(child, "!="));
				break;
			case "GTE":
				children.add(new OperatorModel(child, ">="));
				break;
			case "LTE":
				children.add(new OperatorModel(child, "<="));
			case "NOT":
				children.add(new NOTConditionModel(child, "NOT"));
				break;
			case "Succeeded":
				children.add(new ConditionModel(child, "Succeeded"));
				break;
			case "RepeatCondition":
				children.add(new ConditionModel(child, "Repeat"));
				break;
			case "Node":
				switch( child.getAttribute("NodeType").getValue() ) {
					case "Assignment":
						children.add(new AssignmentNodeModel(child));
						break;
					default: // NodeBody, NodeList
						switch( child.getAttribute("epx").getValue() ) {
							/*
							case "If":
								children.add(new IfNodeModel(child));
								break;
							case "ElseIf":
								children.add(new ElseIfNodeModel(child));
								break;
							case "Then":
								children.add(new ThenNodeModel(child));
								break;
							*/
							case "For":
								children.add(new ForNodeModel(child));
								break;
							case "aux":
								children.add(new AuxNodeModel(child));
								break;
							case "While":
								children.add(new WhileNodeModel(child));
								break;
							case "Condition":
								children.add(new ConditionModel(child));
								break;
							case "Action":
								children.add(new ActionNodeModel(child));
								break;
							case "Sequence":
								for( BaseModel grandchild : child.getChildren() ) {
									generateChild(grandchild);
								}
								break;
							default:
								children.add(new NodeModel(child));
								break;
						}
						break;
				}
				break;
			default:
				System.out.println(child.getName());
				children.add(new BaseModel(child.getOriginalNode(), child.getParent(), child.getOrder()));
				break;
		}
	}
	
	protected String substitute(String nodeId) {
		for( BaseModel child : children ) {
			if( child.hasAttribute("epx") ) {
				if( child.getAttribute("epx").getValue().equals("Condition") ) { 
					if( nodeId.equals(child.getQuality("NodeId").decompile(0)) ) {
						return child.decompile(0);
					}
				}
			}
		}
		return null;
	}
	
	@Override
	public String decompile(int indentLevel) {
		String ret = indent(indentLevel) + getQuality("NodeId").decompile(0) + ":\n{\n";
		for( BaseModel child : children ) {
			if( child instanceof ConditionModel ) {
				continue;
			}
			ret += child.decompile(indentLevel+1) + "\n";
		}
		ret += indent(indentLevel) + "}";
		return ret;
	}
	
}
