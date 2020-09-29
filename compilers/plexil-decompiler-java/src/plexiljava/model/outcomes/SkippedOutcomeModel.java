package plexiljava.model.outcomes;

import plexiljava.model.BaseModel;

public class SkippedOutcomeModel extends OutcomeModel {
    public SkippedOutcomeModel(BaseModel node) {
        super(node, "SKIPPED");
    }
}
