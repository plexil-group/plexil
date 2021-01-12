<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="SafeDrive.ple"><GlobalDeclarations ColNo="0" LineNo="1"><CommandDeclaration ColNo="0" LineNo="1"><Name>Drive</Name><Parameter><Type>Integer</Type></Parameter></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="2"><Name>TakePicture</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="3"><Name>pprint</Name><AnyParameters/></CommandDeclaration><StateDeclaration ColNo="8" LineNo="4"><Name>WheelStuck</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration></GlobalDeclarations><Node NodeType="NodeList" epx="While" ColNo="0" LineNo="6"><NodeId>SafeDrive</NodeId><VariableDeclarations><DeclareVariable ColNo="2" LineNo="8"><Name>pictures</Name><Type>Integer</Type><InitialValue><IntegerValue>0</IntegerValue></InitialValue></DeclareVariable></VariableDeclarations><EndCondition><OR><LookupOnChange><Name><StringValue>WheelStuck</StringValue></Name></LookupOnChange><EQNumeric><IntegerVariable>pictures</IntegerVariable><IntegerValue>10</IntegerValue></EQNumeric></OR></EndCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="aux"><NodeId generated="1">ep2cp_WhileBody</NodeId><RepeatCondition><Succeeded><NodeRef dir="child">ep2cp_WhileTest</NodeRef></Succeeded></RepeatCondition><NodeBody><NodeList><Node NodeType="Empty" epx="Condition"><NodeId generated="1">ep2cp_WhileTest</NodeId><PostCondition><NOT><LookupNow><Name><StringValue>WheelStuck</StringValue></Name></LookupNow></NOT></PostCondition></Node><Node NodeType="NodeList" epx="Action" ColNo="2" LineNo="12"><NodeId>BLOCK__1</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><StartCondition><Succeeded><NodeRef dir="sibling">ep2cp_WhileTest</NodeRef></Succeeded></StartCondition><SkipCondition><PostconditionFailed><NodeRef dir="sibling">ep2cp_WhileTest</NodeRef></PostconditionFailed></SkipCondition><NodeBody><NodeList><Node NodeType="Command" ColNo="4" LineNo="13"><NodeId>OneMeter</NodeId><NodeBody><Command ColNo="16" LineNo="13"><Name><StringValue>Drive</StringValue></Name><Arguments ColNo="22" LineNo="13"><IntegerValue>1</IntegerValue></Arguments></Command></NodeBody></Node><Node NodeType="Command" ColNo="4" LineNo="14"><NodeId>TakePic</NodeId><StartCondition><AND><Finished><NodeRef dir="sibling">OneMeter</NodeRef></Finished><LT><IntegerVariable>pictures</IntegerVariable><IntegerValue>10</IntegerValue></LT></AND></StartCondition><NodeBody><Command ColNo="6" LineNo="16"><Name><StringValue>TakePicture</StringValue></Name></Command></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="18"><NodeId>Counter</NodeId><StartCondition><Finished><NodeRef dir="sibling">TakePic</NodeRef></Finished></StartCondition><PreCondition><LT><IntegerVariable>pictures</IntegerVariable><IntegerValue>10</IntegerValue></LT></PreCondition><NodeBody><Assignment ColNo="6" LineNo="20"><IntegerVariable>pictures</IntegerVariable><NumericRHS><ADD ColNo="26" LineNo="20"><IntegerVariable>pictures</IntegerVariable><IntegerValue>1</IntegerValue></ADD></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Command" ColNo="4" LineNo="22"><NodeId>Print</NodeId><StartCondition><Finished><NodeRef dir="sibling">Counter</NodeRef></Finished></StartCondition><NodeBody><Command ColNo="13" LineNo="22"><Name><StringValue>pprint</StringValue></Name><Arguments ColNo="21" LineNo="22"><StringValue>Pictures taken:</StringValue><IntegerVariable>pictures</IntegerVariable></Arguments></Command></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>