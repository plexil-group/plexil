# IpcAdapter

IpcAdapter was one of the first interface adapters developed for
the PLEXIL Universal Exec.  It uses the TCA IPC package developed at
Carnegie Mellon University.  TCA IPC is a cross-platform
publish-subscribe package that works via shared memory or IP sockets.

IpcAdapter implements commands, lookups, and planner updates.

The `OnCommand` Extended PLEXIL feature uses the ReceiveCommand and
GetParameters commands to allow plans to receive commands from other
agents via IPC.

## Lookups

IpcAdapter supports lookups in two ways:

* Lookup names specified via the LookupNames configuration element,
  and all unassigned lookup names if the DefaultLookupAdapter
  configuration element is present, are treated as external to this
  Exec. When a LookupNow is executed for one of these names, a
  LookupNow message is published via IPC, and the Exec blocks until a
  reply is received. A LookupOnChange provokes an initial LookupNow
  query, and while it is active, any TelemetryValues messages with the
  lookup's state are posted to it.

* Lookup names specified in the ExternalLookups configuration element
  are published to IPC. The UpdateLookup command (see below) publishes
  new values, which are pushed via TelemetryValues messages. LookupNow
  messages received from other agents are replied to with the most
  recent values posted via UpdateLookup. These cached values are also
  available to the hosting Exec as normal lookups.

## Commands

IpcAdapter registers for and implements the following built-in commands:

* SendMessage
* ReceiveMessage
* ReceiveCommand
* GetParameter
* SendReturnValue
* UpdateLookup

Any other commands which IpcAdapter is delegated to handle, either via
the CommandNames configuration element, or all unassigned commands if
the DefaultCommandAdapter configuration element is present, are passed
through and published via IPC via Command messages.

### SendMessage

Command SendMessage(String message);

The SendMessage command does what it says; it publishes a message, the
contents of its parameter, via IPC. If successful, it immediately
sets a `COMMAND_SUCCESS` command handle value.

### ReceiveMessage

String Command ReceiveMessage(String message);

The ReceiveMessage command establishes a handler to subscribe to the
named message, and immediately sets a `COMMAND_SENT_TO_SYSTEM` command
handle value. When the requested message is received via IPC, it is
returned as the value of this command.

### ReceiveCommand

String Command ReceiveCommand(String commandName);

The ReceiveCommand command establishes a handler to subscribe to the
named command, and immediately sets a `COMMAND_SENT_TO_SYSTEM` command
handle value. When the requested command is received via IPC,
ReceiveCommand returns a string with the serial number of the command,
followed by a colon and the ID of the sender. The returned value can
be used to retrieve any parameters sent with the command; see the
GetParameter command.

### GetParameter

Any Command GetParameter(String commandIdentifier, Integer index);

The GetParameter command queries the IpcAdapter for a parameter
associated with a received command (see ReceiveCommand).  If the
command has the requested parameter, GetParameter returns the value of
the parameter as it was sent.. Whether or not the parameter is present,
GetParameter immediately sets a `COMMAND_SENT_TO_SYSTEM` command handle
value.

### SendReturnValue

Command SendReturnValue(String commandIdentifier, Any value);

The SendReturnValue command publishes the given value to IPC as a
return value for the command invocation identified in the
commandIdentifier string. It immediately sets a `COMMAND_SUCCESS` status
and returns no values.

### UpdateLookup

Command UpdateLookup(String stateName, Any value, ...);

If stateName is not declared as an ExternalLookup, the UpdateLookup
command immediately sets a `COMMAND_FAILED` command handle value. If
stateName is so declared, the UpdateLookup command caches the value
for local Lookups, and attempts to publish the new value. If
publication is successful, UpdateLookup sets a `COMMAND_SUCCESS`
command handle value; if publication fails, it sets a `COMMAND_FAILED`
value instead.

## PlannerUpdates

If IpcAdapter is configured with the PlannerUpdate configuration
element, whenever an Update node is executed, the adapter publishes
the node ID and the specified list of name-value pairs.
