package plexiljava.model.functions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;
import plexiljava.model.TypedNodeModel;
import plexiljava.model.operators.OperatorModel;

public class FunctionModel extends TypedNodeModel {

    public FunctionModel(BaseModel node) {
        super(node);
    }

    public FunctionModel(BaseModel node, String type) {
        super(node, type);
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        OperatorModel commas = new OperatorModel(this, ",");
        dsb.append(type, "(", commas.decompile(0), ")");
        return dsb.toString();
    }

}
