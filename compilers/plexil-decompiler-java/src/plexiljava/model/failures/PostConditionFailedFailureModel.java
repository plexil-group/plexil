package plexiljava.model.failures;

import plexiljava.model.BaseModel;

public class PostConditionFailedFailureModel extends FailureModel {
    public PostConditionFailedFailureModel(BaseModel node) {
        super(node, "POST_CONDITION_FAILED");
    }
}
