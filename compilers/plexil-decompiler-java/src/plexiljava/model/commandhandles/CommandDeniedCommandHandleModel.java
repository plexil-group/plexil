package plexiljava.model.commandhandles;

import plexiljava.model.BaseModel;

public class CommandDeniedCommandHandleModel extends CommandHandleModel {
    public CommandDeniedCommandHandleModel(BaseModel node) {
        super(node, "COMMAND_DENIED");
    }
}
