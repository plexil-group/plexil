<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="DriveToSchool.ple"><GlobalDeclarations ColNo="8" LineNo="1"><StateDeclaration ColNo="8" LineNo="1"><Name>at_school</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="2"><Name>no_passengers</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="3"><Name>car_started</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="4"><Name>raining</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><CommandDeclaration ColNo="0" LineNo="6"><Name>depress_accelerator</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="7"><Name>depress_clutch</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="8"><Name>turn_key</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="9"><Name>push_start</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="10"><Name>select_radio</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="11"><Name>select_CD</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="12"><Name>turn_on_wipers</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="13"><Name>turn_on_lights</Name></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="14"><Name>drive_a_bit</Name></CommandDeclaration></GlobalDeclarations><Node NodeType="NodeList" epx="Sequence" ColNo="0" LineNo="16"><NodeId>DriveToSchool</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="Sequence" ColNo="2" LineNo="18"><NodeId>StartCar</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="Try" ColNo="4" LineNo="20"><NodeId>Try__0</NodeId><EndCondition><OR><Succeeded><NodeRef dir="child">KeyStart</NodeRef></Succeeded><Finished><NodeRef dir="child">PushStart</NodeRef></Finished></OR></EndCondition><PostCondition><OR><Succeeded><NodeRef dir="child">KeyStart</NodeRef></Succeeded><Succeeded><NodeRef dir="child">PushStart</NodeRef></Succeeded></OR></PostCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="Sequence" ColNo="6" LineNo="22"><NodeId>KeyStart</NodeId><PostCondition><LookupNow><Name><StringValue>car_started</StringValue></Name></LookupNow></PostCondition><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="For" ColNo="8" LineNo="25"><NodeId>PrimeAccelerator</NodeId><VariableDeclarations><DeclareVariable ColNo="16" LineNo="27"><Name>count</Name><Type>Integer</Type><InitialValue><IntegerValue>0</IntegerValue></InitialValue></DeclareVariable></VariableDeclarations><NodeBody><NodeList><Node NodeType="NodeList" epx="aux"><NodeId generated="1">ep2cp_ForLoop</NodeId><SkipCondition><NOT><LT><IntegerVariable>count</IntegerVariable><IntegerValue>3</IntegerValue></LT></NOT></SkipCondition><RepeatCondition><BooleanValue>true</BooleanValue></RepeatCondition><NodeBody><NodeList><Node NodeType="Command" ColNo="12" LineNo="29"><NodeId>PressAccelerator</NodeId><NodeBody><Command ColNo="32" LineNo="29"><Name><StringValue>depress_accelerator</StringValue></Name></Command></NodeBody></Node><Node NodeType="Assignment" epx="LoopVariableUpdate"><NodeId generated="1">ep2cp_ForLoopUpdater</NodeId><StartCondition><Finished><NodeRef dir="sibling">PressAccelerator</NodeRef></Finished></StartCondition><NodeBody><Assignment><IntegerVariable>count</IntegerVariable><NumericRHS><ADD ColNo="52" LineNo="27"><IntegerVariable>count</IntegerVariable><IntegerValue>1</IntegerValue></ADD></NumericRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="NodeList" epx="Concurrence" ColNo="8" LineNo="33"><NodeId>TurnKeyCombo</NodeId><StartCondition><Finished><NodeRef dir="sibling">PrimeAccelerator</NodeRef></Finished></StartCondition><NodeBody><NodeList><Node NodeType="Command" ColNo="10" LineNo="36"><NodeId>DepressClutch</NodeId><NodeBody><Command ColNo="27" LineNo="36"><Name><StringValue>depress_clutch</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="10" LineNo="37"><NodeId>TurnKey</NodeId><EndCondition><OR><LookupOnChange><Name><StringValue>car_started</StringValue></Name></LookupOnChange><GE><LookupOnChange><Name><StringValue>time</StringValue></Name><Tolerance><RealValue>0.001</RealValue></Tolerance></LookupOnChange><ADD ColNo="72" LineNo="41"><NodeTimepointValue ColNo="48" LineNo="41"><NodeId>TurnKey</NodeId><NodeStateValue>EXECUTING</NodeStateValue><Timepoint>START</Timepoint></NodeTimepointValue><RealValue>10.0</RealValue></ADD></GE></OR></EndCondition><NodeBody><Command ColNo="12" LineNo="42"><Name><StringValue>turn_key</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="Command" ColNo="6" LineNo="47"><NodeId>PushStart</NodeId><StartCondition><Finished><NodeRef dir="sibling">KeyStart</NodeRef></Finished></StartCondition><PostCondition><LookupNow><Name><StringValue>car_started</StringValue></Name></LookupNow></PostCondition><NodeBody><Command ColNo="8" LineNo="50"><Name><StringValue>push_start</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="NodeList" epx="If" ColNo="2" LineNo="55"><NodeId>SelectStation</NodeId><StartCondition><Finished><NodeRef dir="sibling">StartCar</NodeRef></Finished></StartCondition><NodeBody><NodeList><Node NodeType="Command" epx="Then" ColNo="33" LineNo="57"><StartCondition><LookupNow><Name><StringValue>no_passengers</StringValue></Name></LookupNow></StartCondition><SkipCondition><NOT><LookupNow><Name><StringValue>no_passengers</StringValue></Name></LookupNow></NOT></SkipCondition><NodeId>Radio</NodeId><NodeBody><Command ColNo="42" LineNo="57"><Name><StringValue>select_radio</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" epx="Else" ColNo="9" LineNo="58"><StartCondition><NOT><LookupNow><Name><StringValue>no_passengers</StringValue></Name></LookupNow></NOT></StartCondition><SkipCondition><LookupNow><Name><StringValue>no_passengers</StringValue></Name></LookupNow></SkipCondition><NodeId>CD</NodeId><NodeBody><Command ColNo="15" LineNo="58"><Name><StringValue>select_CD</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="NodeList" epx="While" ColNo="2" LineNo="62"><NodeId>DriveUntilAtSchool</NodeId><StartCondition><Finished><NodeRef dir="sibling">SelectStation</NodeRef></Finished></StartCondition><RepeatCondition><Succeeded><NodeRef dir="child">ep2cp_WhileTest</NodeRef></Succeeded></RepeatCondition><NodeBody><NodeList><Node NodeType="Empty" epx="Condition"><NodeId generated="1">ep2cp_WhileTest</NodeId><PostCondition><NOT><LookupNow><Name><StringValue>at_school</StringValue></Name></LookupNow></NOT></PostCondition></Node><Node NodeType="NodeList" epx="Action" ColNo="4" LineNo="65"><NodeId>KeepDriving</NodeId><StartCondition><Succeeded><NodeRef dir="sibling">ep2cp_WhileTest</NodeRef></Succeeded></StartCondition><SkipCondition><PostconditionFailed><NodeRef dir="sibling">ep2cp_WhileTest</NodeRef></PostconditionFailed></SkipCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="If" ColNo="6" LineNo="67"><NodeId>HandleRain</NodeId><NodeBody><NodeList><Node NodeType="NodeList" epx="Then" ColNo="10" LineNo="70"><StartCondition><LookupNow><Name><StringValue>raining</StringValue></Name></LookupNow></StartCondition><SkipCondition><NOT><LookupNow><Name><StringValue>raining</StringValue></Name></LookupNow></NOT></SkipCondition><NodeId>Concurrence__12</NodeId><NodeBody><NodeList><Node NodeType="Command" ColNo="12" LineNo="72"><NodeId>Wipers</NodeId><NodeBody><Command ColNo="22" LineNo="72"><Name><StringValue>turn_on_wipers</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="12" LineNo="73"><NodeId>Lights</NodeId><NodeBody><Command ColNo="22" LineNo="73"><Name><StringValue>turn_on_lights</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="Command" ColNo="6" LineNo="78"><NodeId>DriveABit</NodeId><EndCondition><LookupOnChange><Name><StringValue>at_school</StringValue></Name></LookupOnChange></EndCondition><NodeBody><Command ColNo="8" LineNo="81"><Name><StringValue>drive_a_bit</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>