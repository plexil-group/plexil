<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="AssignToParentExit.ple"><Node NodeType="NodeList" epx="Concurrence" ColNo="0" LineNo="1"><NodeId>AssignToParentExit</NodeId><VariableDeclarations><DeclareVariable ColNo="2" LineNo="4"><Name>step_failed</Name><Type>Boolean</Type><InitialValue><BooleanValue>false</BooleanValue></InitialValue></DeclareVariable></VariableDeclarations><ExitCondition ColNo="2" LineNo="5"><BooleanVariable>step_failed</BooleanVariable></ExitCondition><EndCondition ColNo="2" LineNo="6"><Finished ColNo="6" LineNo="6"><NodeId>Kid</NodeId></Finished></EndCondition><NodeBody><NodeList><Node ColNo="0" LineNo="8" NodeType="Assignment"><NodeId>Kid</NodeId><NodeBody><Assignment ColNo="2" LineNo="9"><BooleanVariable>step_failed</BooleanVariable><BooleanRHS><BooleanValue>true</BooleanValue></BooleanRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>