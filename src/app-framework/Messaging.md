Idea for a generic transport-agnostic messaging interface
---------------------------------------------------------

This is a proposed replacement for the IpcAdapter and UdpAdapter's
notion of commanding from an external agent, informed by an
understanding of their current implementation.

The interface as seen from PLEXIL would be:

* Get notified of a message via Lookup HaveMessage.  Sneak a look at
  the message text via Lookup PeekAtMessage.

* Use the Command GetMessageHandle to get a handle for the message at
  the head of the queue.

* Use Lookups MessageMessage, MessageSender, MessageParameterCount,
  MessageParameter with the handle as a parameter.

* When finished, use the Command ReleaseMessageHandle.

New Lookups:

 Boolean Lookup HaveMessage();

Returns true if a message is enqueued, false otherwise.  Intended to
be used as a LookupOnChange in a StartCondition.

 String Lookup PeekAtMessage();

Return the text of the message at the head of the message queue,
without dequeueing it.  Returns unknown if the queue is empty.
(Optional extension.)

 String Lookup PeekAtMessageSender();

Return the sender of the message at the head of the message queue, if
known, without dequeueing it.  (Unknown sender is a legal value in a
publish/subscribe environment.)  Returns unknown if the queue is
empty.  (Optional extension.)

 String Lookup MessageText(String handle);

Returns the text of the message associated with the handle.  Always
returns the same value as long as the handle is active.  Returns
unknown if handle is not active.

 Integer Lookup MessageParameterCount(String handle);

Return the number of parameters associated with the message handle.
Always returns the same value as long as the handle is active.
Returns unknown if the handle is inactive.

 Any Lookup MessageParameter(String handle, Integer zeroBasedIndex);

Returns the corresponding parameter for the message, if it exists,
irrespective of type.  Returns unknown if the parameter value is
unknown (a legal value), if the index refers to a nonexistent
parameter, or the handle is inactive.  Returns the same value as long
as the handle is active.

 String Lookup MessageSender(String handle);

Return the ID of the sender associated with the message handle, if
known.  (Unknown sender is a legal value in a publish/subscribe
environment.)  Always returns the same value as long as the handle is
active.  Returns unknown if the handle is not active.

 Date Lookup MessageArrived(String handle);

Return the time of arrival of the message associated with the handle,
if known.  Always returns the same value as long as the handle is
active.  Returns unknown if the handle is not active.

New Commands:

 String Command GetMessageHandle;

Returns the handle of the message at the head of the queue, unknown if
queue empty.  Causes the message to be dequeued, and caches the
contents of the message, so they can be accessed via the Lookups
described above.

 String Command ReleaseMessageHandle(String handle);

Tells the interface the message associated with the handle is no
longer needed.  The Lookups associated with this handle become
unknown.

 Command PublishMessage(String message, ....);

Publishes (broadcasts) the message and its associated parameters,
along with the sender ID if the transport supports it.  Sets command
handle to COMMAND_SUCCESS if successfully sent, or
COMMAND_INTERFACE_ERROR if transmission failed in a fashion detectable
at the sending end.

 String Command SendMessage(String recipient, String message, ....);

Sends the message and its associated parameters, along with the sender
ID if the transport supports it, directed to the named recipient.
Sets command handle to COMMAND_SUCCESS if successfully sent, or
COMMAND_INTERFACE_ERROR if transmission failed in a fashion detectable
at the sending end.

The PublishMessage and SendMessage commands are generic enough that
they can be used to return data requested by a previously received
message, e.g. command acknowledgement.

Implementation
--------------

Extend the existing input queue to add a new entry type for messages
received from other agents.  The queue would record the message, any
associated parameters, the time of receipt, and the sender (where
available).

Design, document and implement new interface objects and methods to
implement (at least) the Lookups and Commands described for PLEXIL
plans.

Design, document, and implement a mechanism for populating the state
cache with message data, and for deleting inactive message data from
the PLEXIL state cache.  Turns out this is a bigger job than I had
realized; the state cache actually does the brunt of the work, and
there are some possible implications for the Lookup implementation.

Design, document, and implement a transport-independent API allowing
straightforward connection of the interface adapter to a variety of
transport mechanisms.

Incorporate the above into the PLEXIL app-framework module.

Design, document, and implement reasonable Extended PLEXIL macros and
Standard PLEXIL syntax to facilitate use of these features.
