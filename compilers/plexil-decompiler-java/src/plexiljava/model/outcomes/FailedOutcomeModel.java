package plexiljava.model.outcomes;

import plexiljava.model.BaseModel;

public class FailedOutcomeModel extends OutcomeModel {
    public FailedOutcomeModel(BaseModel node) {
        super(node, "FAILURE");
    }
}
