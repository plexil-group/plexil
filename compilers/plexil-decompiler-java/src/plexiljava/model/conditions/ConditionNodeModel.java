package plexiljava.model.conditions;

import plexiljava.model.BaseModel;

public class ConditionNodeModel extends ConditionModel {

    public ConditionNodeModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !children.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        return children.get(0).decompile(indentLevel);
    }
}
