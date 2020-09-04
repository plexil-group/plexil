package plexiljava.model.states;

import plexiljava.model.BaseModel;

public class InactiveStateModel extends StateModel {
    public InactiveStateModel(BaseModel node) {
        super(node, "INACTIVE");
    }
}