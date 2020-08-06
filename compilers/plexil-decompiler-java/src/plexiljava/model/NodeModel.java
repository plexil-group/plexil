package plexiljava.model;

import java.util.logging.Level;

import plexiljava.decompilation.Decompilable;
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.main.Decompiler;
import plexiljava.model.commands.CommandModel;
import plexiljava.model.commands.CommandNodeModel;
import plexiljava.model.conditions.ConditionNodeModel;
import plexiljava.model.conditions.EndConditionModel;
import plexiljava.model.conditions.InvariantConditionModel;
import plexiljava.model.conditions.NOTConditionModel;
import plexiljava.model.conditions.PostConditionModel;
import plexiljava.model.conditions.PreConditionModel;
import plexiljava.model.conditions.RepeatConditionModel;
import plexiljava.model.conditions.SkipConditionModel;
import plexiljava.model.conditions.StartConditionModel;
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
import plexiljava.model.expressions.NumericRHSModel;
import plexiljava.model.lookups.LookupNowModel;
import plexiljava.model.lookups.LookupOnChangeModel;
import plexiljava.model.lookups.LookupWithFrequencyModel;
import plexiljava.model.operations.ADDOperatorModel;
import plexiljava.model.operations.ANDOperatorModel;
import plexiljava.model.operations.DIVOperatorModel;
import plexiljava.model.operations.EQOperatorModel;
import plexiljava.model.operations.GTEOperatorModel;
import plexiljava.model.operations.GTOperatorModel;
import plexiljava.model.operations.LTEOperatorModel;
import plexiljava.model.operations.LTOperatorModel;
import plexiljava.model.operations.MULOperatorModel;
import plexiljava.model.operations.NEQOperatorModel;
import plexiljava.model.operations.OROperatorModel;
import plexiljava.model.operations.SUBOperatorModel;
import plexiljava.model.references.SucceededModel;
import plexiljava.model.states.ExecutingStateModel;
import plexiljava.model.states.FailingStateModel;
import plexiljava.model.states.FinishedStateModel;
import plexiljava.model.states.FinishingStateModel;
import plexiljava.model.states.InactiveStateModel;
import plexiljava.model.states.IterationEndedStateModel;
import plexiljava.model.states.WaitingStateModel;
import plexiljava.model.tokens.AnyParametersModel;
import plexiljava.model.tokens.ArgumentsModel;
import plexiljava.model.tokens.ArrayValueModel;
import plexiljava.model.tokens.IndexModel;
import plexiljava.model.tokens.NameModel;
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
			case "NodeBody":
				generateChild(child.getChildren().get(0));
				break;
			case "NodeList":
				for( BaseModel grandchild : child.getChildren() ) {
					generateChild(grandchild);
				}
				break;
			/* Commands */
			case "Command":
				children.add(new CommandModel(child));
				break;
			/* Conditions */
			case "EndCondition":
				children.add(new EndConditionModel(child));
				break;
			case "PreCondition":
				children.add(new PreConditionModel(child));
				break;
			case "PostCondition":
				children.add(new PostConditionModel(child));
				break;
			case "RepeatCondition":
				children.add(new RepeatConditionModel(child));
				break;
			case "SkipCondition":
				children.add(new SkipConditionModel(child));
				break;
			case "StartCondition":
				children.add(new StartConditionModel(child));
				break;
			/* Declarations */
			case "Assignment":
				children.add(new AssignmentModel(child));
				break;
			case "CommandDeclaration":
				children.add(new CommandDeclarationModel(child));
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
			case "StateDeclaration":
				children.add(new StateDeclarationModel(child));
				break;
			/* Expressions */
			case "BooleanRHS":
				children.add(new BooleanRHSModel(child));
				break;
			case "ArrayElement":
				children.add(new ArrayElementModel(child));
				break;
			case "NumericRHS":
				children.add(new NumericRHSModel(child));
				break;
				/* Lookups */
			case "LookupOnChange":
				children.add(new LookupOnChangeModel(child));
				break;
			case "LookupNow":
				children.add(new LookupNowModel(child));
				break;
			case "LookupWithFrequency":
				children.add(new LookupWithFrequencyModel(child));
				break;
			case "InvariantCondition":
				children.add(new InvariantConditionModel(child));
				break;
			/* Operations */
			case "ADD":
				children.add(new ADDOperatorModel(child));
				break;
			case "AND":
				children.add(new ANDOperatorModel(child));
				break;
			case "DIV":
				children.add(new DIVOperatorModel(child));
				break;
			case "EQNumeric":
				children.add(new EQOperatorModel(child));
				break;
			case "EQInternal":
				children.add(new EQOperatorModel(child));
				break;
			case "EQString":
				children.add(new EQOperatorModel(child));
				break;
			case "GTE":
				children.add(new GTEOperatorModel(child));
				break;
			case "GT":
				children.add(new GTOperatorModel(child));
				break;
			case "LTE":
				children.add(new LTEOperatorModel(child));
				break;
			case "LT":
				children.add(new LTOperatorModel(child));
				break;
			case "MUL":
				children.add(new MULOperatorModel(child));
				break;
			case "NEQ":
				children.add(new NEQOperatorModel(child));
				break;
			case "OR":
				children.add(new OROperatorModel(child));
				break;
			case "SUB":
				children.add(new SUBOperatorModel(child));
			case "NOT":
				children.add(new NOTConditionModel(child));
				break;
			/* References */
			case "Succeeded":
				children.add(new SucceededModel(child));
				break;
			/* States */
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
			/* Tokens */
			case "AnyParameters":
				children.add(new AnyParametersModel(child));
				break;
			case "Arguments":
				children.add(new ArgumentsModel(child));
				break;
			case "ArrayValue":
				children.add(new ArrayValueModel(child));
				break;
			case "Index":
				children.add(new IndexModel(child));
				break;
			case "Name":
				children.add(new NameModel(child));
				break;
			case "Parameter":
				children.add(new ParameterModel(child));
				break;
			case "Return":
				children.add(new ReturnModel(child));
				break;
			/* Structures */
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
								case "ElseIf":
									children.add(new ElseIfNodeModel(child));
									break;
								case "For":
									children.add(new ForNodeModel(child));
									break;
								case "If":
									children.add(new IfNodeModel(child));
									break;
								case "Then":
									children.add(new ThenNodeModel(child));
									break;
								case "While":
									children.add(new WhileNodeModel(child));
									break;
								case "aux":
									children.add(new AuxNodeModel(child));
									break;
								case "Action":
									children.add(new ActionNodeModel(child));
									break;
								case "Condition":
									children.add(new ConditionNodeModel(child));
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
				if( Decompiler.VERBOSE ) {
					Decompiler.logger.setLevel(Level.WARNING);
					Decompiler.logger.warning("Unrecognized node type: " + child.getName());
				}
				children.add(new BaseModel(child.getOriginalNode(), child.getParent(), child.getOrder()));
				break;
		}
	}
	
	protected String dereference(String nodeId) throws PatternRecognitionFailureException {
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
	public boolean verify() {
		return hasQuality("NodeId");
	}
	
	@Override
	public String translate(int indentLevel) throws PatternRecognitionFailureException {
		DecompilableStringBuilder dsb = new DecompilableStringBuilder();
		dsb.addIndent(indentLevel);
		
		dsb.append(getQuality("NodeId").getValue());
		dsb.addBlockOpener();
		
		for( BaseModel child : children ) {
			if( child instanceof InvariantConditionModel ) {
				continue;
			}
			
			dsb.addLine(child.decompile(indentLevel+1));
		}
		
		dsb.addBlockCloser(indentLevel);
		
		return dsb.toString();
	}
	
}
