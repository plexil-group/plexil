package plexiljava.model;

import java.util.logging.Level;

import plexiljava.decompilation.Decompilable;
import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.main.Decompiler;
import plexiljava.model.commandhandles.CommandAcceptedCommandHandleModel;
import plexiljava.model.commandhandles.CommandDeniedCommandHandleModel;
import plexiljava.model.commandhandles.CommandFailedCommandHandleModel;
import plexiljava.model.commandhandles.CommandRcvdBySystemCommandHandleModel;
import plexiljava.model.commandhandles.CommandSentToSystemCommandHandleModel;
import plexiljava.model.commandhandles.CommandSuccessCommandHandleModel;
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
import plexiljava.model.containers.ActionNodeModel;
import plexiljava.model.containers.AssignmentNodeModel;
import plexiljava.model.containers.AuxNodeModel;
import plexiljava.model.containers.ConcurrenceNodeModel;
import plexiljava.model.containers.ElseIfNodeModel;
import plexiljava.model.containers.ElseNodeModel;
import plexiljava.model.containers.EmptyNodeModel;
import plexiljava.model.containers.ForNodeModel;
import plexiljava.model.containers.IfNodeModel;
import plexiljava.model.containers.LoopVariableUpdateNodeModel;
import plexiljava.model.containers.ThenNodeModel;
import plexiljava.model.containers.TryNodeModel;
import plexiljava.model.containers.WhileNodeModel;
import plexiljava.model.declarations.AssignmentModel;
import plexiljava.model.declarations.CommandDeclarationModel;
import plexiljava.model.declarations.DeclareArrayModel;
import plexiljava.model.declarations.DeclareVariableModel;
import plexiljava.model.declarations.InitialValueModel;
import plexiljava.model.declarations.LibraryNodeDeclarationModel;
import plexiljava.model.declarations.StateDeclarationModel;
import plexiljava.model.expressions.ArrayElementModel;
import plexiljava.model.expressions.ArrayRHSModel;
import plexiljava.model.expressions.BooleanRHSModel;
import plexiljava.model.expressions.NodeTimepointValue;
import plexiljava.model.expressions.NumericRHSModel;
import plexiljava.model.expressions.StringRHSModel;
import plexiljava.model.external.LibraryNodeCallModel;
import plexiljava.model.external.UpdateModel;
import plexiljava.model.failures.InvariantConditionFailedFailureModel;
import plexiljava.model.failures.ParentFailedFailureModel;
import plexiljava.model.failures.PostConditionFailedFailureModel;
import plexiljava.model.failures.PreConditionFailedFailureModel;
import plexiljava.model.functions.ABSFunctionModel;
import plexiljava.model.functions.MAXFunctionModel;
import plexiljava.model.functions.MINFunctionModel;
import plexiljava.model.functions.SQRTFunctionModel;
import plexiljava.model.lookups.LookupNowModel;
import plexiljava.model.lookups.LookupOnChangeModel;
import plexiljava.model.lookups.LookupWithFrequencyModel;
import plexiljava.model.operators.ADDOperatorModel;
import plexiljava.model.operators.ANDOperatorModel;
import plexiljava.model.operators.DIVOperatorModel;
import plexiljava.model.operators.EQOperatorModel;
import plexiljava.model.operators.GTEOperatorModel;
import plexiljava.model.operators.GTOperatorModel;
import plexiljava.model.operators.LTEOperatorModel;
import plexiljava.model.operators.LTOperatorModel;
import plexiljava.model.operators.MODOperatorModel;
import plexiljava.model.operators.MULOperatorModel;
import plexiljava.model.operators.NEQOperatorModel;
import plexiljava.model.operators.OROperatorModel;
import plexiljava.model.operators.SUBOperatorModel;
import plexiljava.model.outcomes.FailedOutcomeModel;
import plexiljava.model.outcomes.FailureOutcomeModel;
import plexiljava.model.outcomes.SkippedOutcomeModel;
import plexiljava.model.outcomes.SuccessOutcomeModel;
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
import plexiljava.model.tokens.InModel;
import plexiljava.model.tokens.InOutModel;
import plexiljava.model.tokens.IndexModel;
import plexiljava.model.tokens.InterfaceModel;
import plexiljava.model.tokens.IsKnownModel;
import plexiljava.model.tokens.MinusInfinityModel;
import plexiljava.model.tokens.NameModel;
import plexiljava.model.tokens.NodeFailureVariableModel;
import plexiljava.model.tokens.NodeOutcomeVariableModel;
import plexiljava.model.tokens.NodeStateVariableModel;
import plexiljava.model.tokens.OutModel;
import plexiljava.model.tokens.PairModel;
import plexiljava.model.tokens.ParameterModel;
import plexiljava.model.tokens.PlusInfinityModel;
import plexiljava.model.tokens.ReturnModel;
import plexiljava.model.tokens.StringValueModel;
import plexiljava.model.tokens.SucceededModel;
import plexiljava.model.tokens.ToleranceModel;

public class NodeModel extends BaseModel implements Decompilable {

    /**
     * A generic extension of the BaseModel that decompiles to a generic PLEXIL node
     * @param node BaseModel to be built off of
     */
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

    /**
     * Generates children of the appropriate type based on the child's qualities
     * Add cases here for new nodes that are added to the model
     * @param child BaseModel to categorize and initialize appropriately
     */
    public void generateChild(BaseModel child) {
        /** Put cases here based on the element's tag in the XML file **/
        switch( child.getName() ) {
        // Special Cases
        case "#comment":
            break;
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
            /* CommandHandles */
        case "CommandAccepted":
            children.add(new CommandAcceptedCommandHandleModel(child));
            break;
        case "CommandDenied":
            children.add(new CommandDeniedCommandHandleModel(child));
            break;
        case "CommandFailed":
            children.add(new CommandFailedCommandHandleModel(child));
            break;
        case "CommandRcvdBySystem":
            children.add(new CommandRcvdBySystemCommandHandleModel(child));
            break;
        case "CommandSentToSystem":
            children.add(new CommandSentToSystemCommandHandleModel(child));
            break;
        case "CommandSuccessCommandHandleModel":
            children.add(new CommandSuccessCommandHandleModel(child));
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
        case "LibraryNodeDeclaration":
            children.add(new LibraryNodeDeclarationModel(child));
            break;
        case "StateDeclaration":
            children.add(new StateDeclarationModel(child));
            break;
            /* Expressions */
        case "ArrayElement":
            children.add(new ArrayElementModel(child));
            break;
        case "ArrayRHS":
            children.add(new ArrayRHSModel(child));
            break;
        case "BooleanRHS":
            children.add(new BooleanRHSModel(child));
            break;
        case "NodeTimepointValue":
            children.add(new NodeTimepointValue(child));
            break;
        case "NumericRHS":
            children.add(new NumericRHSModel(child));
            break;
        case "StringRHS":
            children.add(new StringRHSModel(child));
            break;
            /* External */
        case "LibraryNodeCall":
            children.add(new LibraryNodeCallModel(child));
            break;
        case "Update":
            children.add(new UpdateModel(child));
            break;
            /* Failures */
        case "InvariantconditionFailed":
            children.add(new InvariantConditionFailedFailureModel(child));
            break;
        case "Parentfailed":
            children.add(new ParentFailedFailureModel(child));
            break;
        case "PostconditionFailed":
            children.add(new PostConditionFailedFailureModel(child));
            break;
        case "PreconditionFailed":
            children.add(new PreConditionFailedFailureModel(child));
            break;
            /* Functions */
        case "ABS":
            children.add(new ABSFunctionModel(child));
            break;
        case "MAX":
            children.add(new MAXFunctionModel(child));
            break;
        case "MIN":
            children.add(new MINFunctionModel(child));
            break;
        case "SQRT":
            children.add(new SQRTFunctionModel(child));
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
        case "Concat":
            children.add(new ADDOperatorModel(child));
            break;
        case "DIV":
            children.add(new DIVOperatorModel(child));
            break;
        case "EQBoolean":
            children.add(new EQOperatorModel(child));
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
        case "GE":
            children.add(new GTEOperatorModel(child));
            break;
        case "GT":
            children.add(new GTOperatorModel(child));
            break;
        case "LE":
            children.add(new LTEOperatorModel(child));
            break;
        case "LT":
            children.add(new LTOperatorModel(child));
            break;
        case "MOD":
            children.add(new MODOperatorModel(child));
            break;
        case "MUL":
            children.add(new MULOperatorModel(child));
            break;
        case "NEQ":
            children.add(new NEQOperatorModel(child));
            break;
        case "NEExpression":
            children.add(new NEQOperatorModel(child));
            break;
        case "NEBoolean":
            children.add(new NEQOperatorModel(child));
            break;
        case "NENumeric":
            children.add(new NEQOperatorModel(child));
            break;
        case "NEInternal":
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
            /* Outcomes */
        case "Success":
            children.add(new SuccessOutcomeModel(child));
            break;
        case "Failed":
            children.add(new FailedOutcomeModel(child));
            break;
        case "Failure":
            children.add(new FailureOutcomeModel(child));
            break;
        case "Skipped":
            children.add(new SkippedOutcomeModel(child));
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
        case "NodeStateVariable":
            children.add(new NodeStateVariableModel(child));
            break;
        case "Waiting":
            children.add(new WaitingStateModel(child));
            break;
            /* Tokens */
        case "Alias":
            children.add(new PairModel(child));
            break;
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
        case "In":
            children.add(new InModel(child));
            break;
        case "InOut":
            children.add(new InOutModel(child));
            break;
        case "Interface":
            children.add(new InterfaceModel(child));
            break;
        case "IsKnown":
            children.add(new IsKnownModel(child));
            break;
        case "MinusInfinity":
            children.add(new MinusInfinityModel(child));
            break;
        case "Name":
            children.add(new NameModel(child));
            break;
        case "NodeOutcomeVariable":
            children.add(new NodeOutcomeVariableModel(child));
            break;
        case "NodeFailureVariable":
            children.add(new NodeFailureVariableModel(child));
            break;
        case "Out":
            children.add(new OutModel(child));
            break;
        case "Pair":
            children.add(new PairModel(child));
            break;
        case "Parameter":
            children.add(new ParameterModel(child));
            break;
        case "PlusInfinity":
            children.add(new PlusInfinityModel(child));
            break;
        case "Return":
            children.add(new ReturnModel(child));
            break;
        case "StringValue":
            children.add(new StringValueModel(child));
            break;
        case "Succeeded":
            children.add(new SucceededModel(child));
            break;
        case "Tolerance":
            children.add(new ToleranceModel(child));
            break;
            /** Put cases here based on the XML element's NodeType attribute **/
            /* Structures */
        case "Node":
            switch( child.getAttribute("NodeType").getValue() ) {
            case "Assignment":
                if( !child.hasAttribute("epx") ) {
                    children.add(new AssignmentNodeModel(child));
                    break;
                }
            case "Empty":
                if( !child.hasAttribute("epx") ) {
                    children.add(new EmptyNodeModel(child));
                    break;
                }
            default: // NodeBody, NodeList
                if( child.hasAttribute("epx") ) {
                    /** Put cases here based on the XML element's epx attribute **/
                    switch( child.getAttribute("epx").getValue() ) {
                    case "Sequence":
                        children.add(new NodeModel(child));
                        break;
                        /*
									if( getParent() == null || getParent() instanceof TryNodeModel ) {
										children.add(new NodeModel(child));
										break;
									}
									for( BaseModel grandchild : child.getChildren() ) {
										generateChild(grandchild);
									}
									break;
                         */
                    case "Concurrence":
                        children.add(new ConcurrenceNodeModel(child));
                        break;
                    case "ElseIf":
                        children.add(new ElseIfNodeModel(child));
                        break;
                    case "Else":
                        children.add(new ElseNodeModel(child));
                        break;
                    case "For":
                        children.add(new ForNodeModel(child));
                        break;
                    case "If":
                        children.add(new IfNodeModel(child));
                        break;
                    case "LoopVariableUpdate":
                        children.add(new LoopVariableUpdateNodeModel(child));
                        break;
                    case "Then":
                        children.add(new ThenNodeModel(child));
                        break;
                    case "While":
                        children.add(new WhileNodeModel(child));
                        break;
                    case "Try":
                        children.add(new TryNodeModel(child));
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

    /**
     * Links referenced nodes with their contents
     * @param nodeId String ID of the node being referenced
     * @return String contents of the referenced node
     * @throws PatternRecognitionFailureException if decompilation of the referenced node fails
     */
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
        return /*hasQuality("NodeId");*/ true;
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);

        dsb.append(getQuality("NodeId").getValue());
        dsb.addBlockOpener("Concurrence");

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
