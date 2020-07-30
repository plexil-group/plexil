package plexiljava.model;

import plexiljava.decompilation.Decompilable;
import plexiljava.model.commands.CommandModel;
import plexiljava.model.commands.CommandNodeModel;
import plexiljava.model.conditions.ConditionModel;
import plexiljava.model.conditions.NOTConditionModel;
import plexiljava.model.containers.AssignmentNodeModel;
import plexiljava.model.containers.ConcurrenceNodeModel;
import plexiljava.model.containers.ElseIfNodeModel;
import plexiljava.model.containers.EmptyNodeModel;
import plexiljava.model.containers.ForNodeModel;
import plexiljava.model.containers.IfNodeModel;
import plexiljava.model.containers.ThenNodeModel;
import plexiljava.model.containers.WhileNodeModel;
import plexiljava.model.declarations.AssignmentModel;
import plexiljava.model.declarations.CommandDeclarationModel;
import plexiljava.model.declarations.DeclareArrayModel;
import plexiljava.model.declarations.DeclareVariableModel;
import plexiljava.model.declarations.InitialValueModel;
import plexiljava.model.declarations.StateDeclarationModel;
import plexiljava.model.expressions.ArrayElementModel;
import plexiljava.model.expressions.BooleanRHSModel;
import plexiljava.model.expressions.LookupNowModel;
import plexiljava.model.expressions.LookupOnChangeModel;
import plexiljava.model.expressions.NumericRHSModel;
import plexiljava.model.operations.OperatorModel;
import plexiljava.model.states.ExecutingStateModel;
import plexiljava.model.states.FailingStateModel;
import plexiljava.model.states.FinishedStateModel;
import plexiljava.model.states.FinishingStateModel;
import plexiljava.model.states.InactiveStateModel;
import plexiljava.model.states.IterationEndedStateModel;
import plexiljava.model.states.WaitingStateModel;
import plexiljava.model.tokens.ParameterModel;
import plexiljava.model.tokens.ReturnModel;

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
			case "GlobalDeclarations":
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
			case "CommandDeclaration":
				children.add(new CommandDeclarationModel(child));
				break;
			case "StateDeclaration":
				children.add(new StateDeclarationModel(child));
				break;
			case "LookupOnChange":
				children.add(new LookupOnChangeModel(child));
				break;
			case "LookupNow":
				children.add(new LookupNowModel(child));
				break;
			case "InitialValue":
				children.add(new InitialValueModel(child));
				break;
			case "Command":
				children.add(new CommandModel(child));
				break;
			case "Parameter":
				children.add(new ParameterModel(child));
				break;
			case "Return":
				children.add(new ReturnModel(child));
				break;
			case "InvariantCondition":
				children.add(new ConditionModel(child, "Invariant"));
				break;
			case "StartCondition":
				children.add(new ConditionModel(child, "Start"));
				break;
			case "EndCondition":
				children.add(new ConditionModel(child, "End"));
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
			case "BooleanRHS":
				children.add(new BooleanRHSModel(child));
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
				children.add(new NOTConditionModel(child));
				break;
			case "Succeeded":
				children.add(new ConditionModel(child, "Succeeded"));
				break;
			case "RepeatCondition":
				children.add(new ConditionModel(child, "Repeat"));
				break;
			case "Executing":
				children.add(new ExecutingStateModel(child));
				break;
			case "Failing":
				children.add(new FailingStateModel(child));
				break;
			case "Finished":
				children.add(new FinishedStateModel(child));
				break;
			case "Finishing":
				children.add(new FinishingStateModel(child));
				break;
			case "Inactive":
				children.add(new InactiveStateModel(child));
				break;
			case "IterationEnded":
				children.add(new IterationEndedStateModel(child));
				break;
			case "Waiting":
				children.add(new WaitingStateModel(child));
				break;
			case "Node":
				switch( child.getAttribute("NodeType").getValue() ) {
					case "Assignment":
						children.add(new AssignmentNodeModel(child));
						break;
					case "Empty":
						if( !child.hasAttribute("epx") ) {
							children.add(new EmptyNodeModel(child));
							break;
						}
					default: // NodeBody, NodeList
						if( child.hasAttribute("epx") ) {
							switch( child.getAttribute("epx").getValue() ) {
								case "If":
									children.add(new IfNodeModel(child));
									break;
								case "ElseIf":
									children.add(new ElseIfNodeModel(child));
									break;
								case "Then":
									children.add(new ThenNodeModel(child));
									break;
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
									if( getParent() == null ) {
										children.add(new NodeModel(child));
										break;
									}
									for( BaseModel grandchild : child.getChildren() ) {
										generateChild(grandchild);
									}
									break;
								case "Concurrence":
									children.add(new ConcurrenceNodeModel(child));
									break;
								default:
									children.add(new NodeModel(child));
									break;
							}
						} else {
							switch( child.getAttribute("NodeType").getValue() ) {
								case "Command":
									children.add(new CommandNodeModel(child));
									break;
								default:
									children.add(new NodeModel(child));
									break;
							}
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
