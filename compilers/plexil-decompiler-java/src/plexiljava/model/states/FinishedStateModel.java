package plexiljava.model.states;

import plexiljava.model.BaseModel;

public class FinishedStateModel extends StateModel {
    public FinishedStateModel(BaseModel node) {
        super(node, "FINISHED");
    }
}