package plexiljava.model.states;

import plexiljava.model.BaseModel;

public class FailingStateModel extends StateModel {
    public FailingStateModel(BaseModel node) {
        super(node, "FAILING");
    }
}
