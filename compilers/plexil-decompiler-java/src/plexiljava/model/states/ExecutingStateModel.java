package plexiljava.model.states;

import plexiljava.model.BaseModel;

public class ExecutingStateModel extends StateModel {
    public ExecutingStateModel(BaseModel node) {
        super(node, "EXECUTING");
    }
}