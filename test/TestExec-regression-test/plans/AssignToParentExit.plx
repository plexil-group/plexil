<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator" FileName="plans/AssignToParentExit.ple"><Node NodeType="NodeList" epx="Concurrence" LineNo="3" ColNo="0"><NodeId>AssignToParentExit</NodeId><VariableDeclarations><DeclareVariable LineNo="4" ColNo="2"><Name>step_failed</Name><Type>Boolean</Type><InitialValue><BooleanValue>false</BooleanValue></InitialValue></DeclareVariable></VariableDeclarations><ExitCondition><BooleanVariable>step_failed</BooleanVariable></ExitCondition><EndCondition><EQInternal><NodeStateVariable><NodeId>Kid</NodeId></NodeStateVariable><NodeStateValue>FINISHED</NodeStateValue></EQInternal></EndCondition><NodeBody><NodeList><Node NodeType="Assignment" LineNo="10" ColNo="2"><NodeId>Kid</NodeId><NodeBody><Assignment><BooleanVariable>step_failed</BooleanVariable><BooleanRHS><BooleanValue>true</BooleanValue></BooleanRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>