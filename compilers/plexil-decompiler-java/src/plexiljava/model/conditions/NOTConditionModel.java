package plexiljava.model.conditions;

import plexiljava.decompilation.DecompilableStringBuilder;
import plexiljava.model.BaseModel;

public class NOTConditionModel extends ConditionModel {

    public NOTConditionModel(BaseModel node) {
        super(node, "");
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        DecompilableStringBuilder dsb = new DecompilableStringBuilder();
        dsb.addIndent(indentLevel);
        dsb.append("NOT (", super.translate(0), ")");
        return dsb.toString();
    }

}
