<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="plans/isKnown1.ple"><Node NodeType="NodeList" epx="Concurrence" LineNo="1" ColNo="0"><NodeId>isKnown1</NodeId><VariableDeclarations><DeclareVariable LineNo="4" ColNo="4"><Name>test</Name><Type>Boolean</Type></DeclareVariable></VariableDeclarations><PostCondition><IsKnown><NodeStateVariable><NodeId>two</NodeId></NodeStateVariable></IsKnown></PostCondition><NodeBody><NodeList><Node NodeType="Empty" LineNo="8" ColNo="2"><NodeId>two</NodeId><StartCondition><IsKnown><BooleanVariable>test</BooleanVariable></IsKnown></StartCondition></Node><Node NodeType="Assignment" LineNo="13" ColNo="2"><NodeId>one</NodeId><NodeBody><Assignment LineNo="14" ColNo="4"><BooleanVariable>test</BooleanVariable><BooleanRHS><BooleanValue>true</BooleanValue></BooleanRHS></Assignment></NodeBody></Node><Node NodeType="Empty" LineNo="16" ColNo="2"><NodeId>three</NodeId><StartCondition><IsKnown><NodeOutcomeVariable><NodeId>two</NodeId></NodeOutcomeVariable></IsKnown></StartCondition></Node></NodeList></NodeBody></Node></PlexilPlan>