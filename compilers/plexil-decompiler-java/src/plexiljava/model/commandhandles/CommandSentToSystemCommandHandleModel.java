package plexiljava.model.commandhandles;

import plexiljava.model.BaseModel;

public class CommandSentToSystemCommandHandleModel extends CommandHandleModel {
    public CommandSentToSystemCommandHandleModel(BaseModel node) {
        super(node, "COMMAND_SENT_TO_SYSTEM");
    }
}
