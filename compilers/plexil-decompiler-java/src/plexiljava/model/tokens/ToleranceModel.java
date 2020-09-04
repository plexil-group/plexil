package plexiljava.model.tokens;

import plexiljava.model.BaseModel;
import plexiljava.model.NodeModel;

public class ToleranceModel extends NodeModel {

    public ToleranceModel(BaseModel node) {
        super(node);
    }

    @Override
    public boolean verify() {
        return !qualities.isEmpty();
    }

    @Override
    public String translate(int indentLevel) throws PatternRecognitionFailureException {
        return qualities.get(0).decompile(indentLevel);
    }

}
