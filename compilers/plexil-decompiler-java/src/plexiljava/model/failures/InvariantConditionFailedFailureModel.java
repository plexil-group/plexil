package plexiljava.model.failures;

import plexiljava.model.BaseModel;

public class InvariantConditionFailedFailureModel extends FailureModel {
    public InvariantConditionFailedFailureModel(BaseModel node) {
        super(node, "INVARIANT_CONDITION_FAILED");
    }
}
