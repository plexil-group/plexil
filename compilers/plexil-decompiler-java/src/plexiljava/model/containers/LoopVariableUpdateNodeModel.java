package plexiljava.model.containers;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;
import plexiljava.model.declarations.AssignmentModel;

public class LoopVariableUpdateNodeModel extends NodeModel {

    public LoopVariableUpdateNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return hasChild(AssignmentModel.class);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        String dcmp = getChild(AssignmentModel.class).decompile(0);
        dcmp = dcmp.replaceAll(".*= ", "");
        dsb.append(dcmp);
        return dsb.toString();
    }

}
