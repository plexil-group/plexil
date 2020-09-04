package plexiljava.model.failures;

import plexiljava.model.BaseModel;

public class ParentFailedFailureModel extends FailureModel {
    public ParentFailedFailureModel(BaseModel node) {
        super(node, "PARENT_FAILED");
    }
}
