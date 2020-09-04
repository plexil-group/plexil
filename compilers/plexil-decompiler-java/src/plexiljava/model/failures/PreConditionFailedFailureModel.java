package plexiljava.model.failures;

import plexiljava.model.BaseModel;

public class PreConditionFailedFailureModel extends FailureModel {
    public PreConditionFailedFailureModel(BaseModel node) {
        super(node, "PRE_CONDITION_FAILED");
    }
}
