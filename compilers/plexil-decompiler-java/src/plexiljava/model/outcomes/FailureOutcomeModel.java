package plexiljava.model.outcomes;

import plexiljava.model.BaseModel;

public class FailureOutcomeModel extends OutcomeModel {
    public FailureOutcomeModel(BaseModel node) {
        super(node, "FAILURE");
    }
}
