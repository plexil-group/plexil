# (Proposed) Simulation Script Format

The Plexil Simulator can generate responses to Plexil commands, and
send telemetry for lookups, according to a simple script.

## Script elements

Scripts consist of two sections:

* *Declarations*
* *Script actions*

### Declarations

Commands and Lookups must be explicitly declared before use. The
syntax is very similar to Standard Plexil declarations.

* Return type names

    The standard type names in Standard Plexil are:

        * Boolean
        * Integer
        * Real
        * String
        * BooleanArray
        * IntegerArray
        * RealArray
        * StringArray

* Commands

        [<return-type>] Command <name>

    Examples of commands which return no values:

        Command HaltAndCatchFire
        Command ReadTheNews
    
    Examples of commands which return values:

        String Command SaveDatabase
        Integer Command CountExternalConnections

* Lookups

        <return-type> Lookup <name>

    Examples of lookups:

        Integer Lookup TempInDegreesF
        RealArray GetRoverLocation

### Script actions

Script actions come in two forms: *command* and *telemetry* actions.

* The syntax of a command action is:

        <cmd-name> <index> <#responses> <delay>
        <return-val>

    Where ```<index>``` is the number of the command to which this
    action applies (0 = default), ```<#responses>``` ???,
    ```<delay>``` is the number of seconds after receiving the command
    that the response will be sent, and ```<return-val>``` is a
    literal value, parsed as the declared return type for the command
    or lookup, or as a *command handle value*.

    Whether or not a command has a declared return value, commands are
    also expected to return a command handle value, effectively a
    command status. Valid values are:

        * COMMAND_SENT_TO_SYSTEM
        * COMMAND_ACCEPTED
        * COMMAND_RCVD_BY_SYSTEM
        * COMMAND_FAILED
        * COMMAND_DENIED
        * COMMAND_SUCCESS
        
    Values of ```COMMAND_FAILED```, ```COMMAND_DENIED```, and
    ```COMMAND_SUCCESS``` effectively end execution of the Command
    node. The other values can be sent as intermediate status if
    desired.

* The syntax of a telemetry action is:

        <lkup-name> <time>
        <return-val>

    Where ```<time>``` is the number of seconds after simulator start
    that the response will be sent, and ```<return-val>``` is a
    literal value, parsed as the declared return type for the command
    or lookup.
