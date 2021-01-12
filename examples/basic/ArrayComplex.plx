<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="ArrayComplex.ple"><Node NodeType="NodeList" epx="Sequence" ColNo="0" LineNo="1"><NodeId>ArrayComplex</NodeId><VariableDeclarations><DeclareArray ColNo="2" LineNo="3"><Name>foo</Name><Type>Real</Type><MaxSize>4</MaxSize><InitialValue><ArrayValue Type="Real"><RealValue>0.0</RealValue><RealValue>0.0</RealValue><RealValue>0.0</RealValue><RealValue>0.0</RealValue></ArrayValue></InitialValue></DeclareArray><DeclareArray ColNo="2" LineNo="4"><Name>bar</Name><Type>Real</Type><MaxSize>30</MaxSize></DeclareArray></VariableDeclarations><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="For" ColNo="2" LineNo="6"><NodeId>Initialize</NodeId><VariableDeclarations><DeclareVariable ColNo="19" LineNo="6"><Name>i</Name><Type>Integer</Type><InitialValue><IntegerValue>0</IntegerValue></InitialValue></DeclareVariable></VariableDeclarations><NodeBody><NodeList><Node NodeType="NodeList" epx="aux"><NodeId generated="1">ep2cp_ForLoop</NodeId><SkipCondition><NOT><LT><IntegerVariable>i</IntegerVariable><IntegerValue>30</IntegerValue></LT></NOT></SkipCondition><RepeatCondition><BooleanValue>true</BooleanValue></RepeatCondition><NodeBody><NodeList><Node NodeType="Assignment" ColNo="50" LineNo="6"><NodeId>BarInit</NodeId><NodeBody><Assignment ColNo="59" LineNo="6"><ArrayElement><ArrayVariable>bar</ArrayVariable><Index><IntegerVariable>i</IntegerVariable></Index></ArrayElement><NumericRHS><RealValue>0.0</RealValue></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" epx="LoopVariableUpdate"><NodeId generated="1">ep2cp_ForLoopUpdater</NodeId><StartCondition><Finished><NodeRef dir="sibling">BarInit</NodeRef></Finished></StartCondition><NodeBody><Assignment><IntegerVariable>i</IntegerVariable><NumericRHS><ADD ColNo="44" LineNo="6"><IntegerVariable>i</IntegerVariable><IntegerValue>1</IntegerValue></ADD></NumericRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="NodeList" epx="For" ColNo="2" LineNo="8"><NodeId>for__0</NodeId><VariableDeclarations><DeclareVariable ColNo="7" LineNo="8"><Name>i</Name><Type>Real</Type><InitialValue><RealValue>0.0</RealValue></InitialValue></DeclareVariable></VariableDeclarations><StartCondition><Finished><NodeRef dir="sibling">Initialize</NodeRef></Finished></StartCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="aux"><NodeId generated="1">ep2cp_ForLoop</NodeId><SkipCondition><NOT><LT><RealVariable>i</RealVariable><RealValue>10.0</RealValue></LT></NOT></SkipCondition><RepeatCondition><BooleanValue>true</BooleanValue></RepeatCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="Sequence" ColNo="2" LineNo="9"><NodeId>BLOCK__1</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="Assignment" ColNo="4" LineNo="16"><NodeId>SimpleArrayAssignment</NodeId><NodeBody><Assignment ColNo="28" LineNo="16"><ArrayElement><ArrayVariable>foo</ArrayVariable><Index><IntegerValue>1</IntegerValue></Index></ArrayElement><NumericRHS><ADD ColNo="44" LineNo="16"><ArrayElement><ArrayVariable>foo</ArrayVariable><Index><IntegerValue>1</IntegerValue></Index></ArrayElement><RealValue>1.0</RealValue></ADD></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="17"><NodeId>SimpleArrayAssignment2</NodeId><StartCondition><Finished><NodeRef dir="sibling">SimpleArrayAssignment</NodeRef></Finished></StartCondition><NodeBody><Assignment ColNo="28" LineNo="17"><ArrayElement><ArrayVariable>bar</ArrayVariable><Index><IntegerValue>2</IntegerValue></Index></ArrayElement><NumericRHS><ADD ColNo="44" LineNo="17"><ArrayElement><ArrayVariable>bar</ArrayVariable><Index><IntegerValue>2</IntegerValue></Index></ArrayElement><RealValue>2.0</RealValue></ADD></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="18"><NodeId>SimpleArrayAssignment3</NodeId><StartCondition><Finished><NodeRef dir="sibling">SimpleArrayAssignment2</NodeRef></Finished></StartCondition><NodeBody><Assignment ColNo="28" LineNo="18"><ArrayElement><ArrayVariable>foo</ArrayVariable><Index><IntegerValue>3</IntegerValue></Index></ArrayElement><NumericRHS><ADD ColNo="44" LineNo="18"><ArrayElement><ArrayVariable>foo</ArrayVariable><Index><IntegerValue>3</IntegerValue></Index></ArrayElement><RealValue>3.0</RealValue></ADD></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="19"><NodeId>SimpleArrayAssignment4</NodeId><StartCondition><Finished><NodeRef dir="sibling">SimpleArrayAssignment3</NodeRef></Finished></StartCondition><NodeBody><Assignment ColNo="28" LineNo="19"><ArrayElement><ArrayVariable>bar</ArrayVariable><Index><IntegerValue>15</IntegerValue></Index></ArrayElement><NumericRHS><ADD ColNo="46" LineNo="19"><ArrayElement><ArrayVariable>bar</ArrayVariable><Index><IntegerValue>15</IntegerValue></Index></ArrayElement><RealValue>4.0</RealValue></ADD></NumericRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="Assignment" epx="LoopVariableUpdate"><NodeId generated="1">ep2cp_ForLoopUpdater</NodeId><StartCondition><Finished><NodeRef dir="sibling">BLOCK__1</NodeRef></Finished></StartCondition><NodeBody><Assignment><RealVariable>i</RealVariable><NumericRHS><ADD ColNo="33" LineNo="8"><RealVariable>i</RealVariable><IntegerValue>1</IntegerValue></ADD></NumericRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>