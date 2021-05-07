package plexiljava.model.states;

import plexiljava.model.BaseModel;

public class WaitingStateModel extends StateModel {
    public WaitingStateModel(BaseModel node) {
        super(node, "WAITING");
    }
}
