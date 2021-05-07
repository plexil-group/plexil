<?xml version="1.0" encoding="UTF-8"?><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="TestUnknown.ple"><GlobalDeclarations ColNo="5" LineNo="1"><StateDeclaration ColNo="5" LineNo="1"><Name>r1</Name><Return><Name>_return_0</Name><Type>Real</Type></Return></StateDeclaration><StateDeclaration ColNo="5" LineNo="2"><Name>r2</Name><Return><Name>_return_0</Name><Type>Real</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="3"><Name>i1</Name><Return><Name>_return_0</Name><Type>Integer</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="4"><Name>i2</Name><Return><Name>_return_0</Name><Type>Integer</Type></Return></StateDeclaration><StateDeclaration ColNo="7" LineNo="5"><Name>s1</Name><Return><Name>_return_0</Name><Type>String</Type></Return></StateDeclaration><StateDeclaration ColNo="7" LineNo="6"><Name>s2</Name><Return><Name>_return_0</Name><Type>String</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="7"><Name>b1</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><StateDeclaration ColNo="8" LineNo="8"><Name>b2</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></StateDeclaration><CommandDeclaration ColNo="7" LineNo="10"><Name>get_string</Name><Return><Name>_return_0</Name><Type>String</Type></Return></CommandDeclaration><CommandDeclaration ColNo="8" LineNo="11"><Name>get_int</Name><Return><Name>_return_0</Name><Type>Integer</Type></Return></CommandDeclaration><CommandDeclaration ColNo="5" LineNo="12"><Name>get_real</Name><Return><Name>_return_0</Name><Type>Real</Type></Return></CommandDeclaration><CommandDeclaration ColNo="8" LineNo="13"><Name>get_bool</Name><Return><Name>_return_0</Name><Type>Boolean</Type></Return></CommandDeclaration><CommandDeclaration ColNo="0" LineNo="14"><Name>pprint</Name><AnyParameters/></CommandDeclaration></GlobalDeclarations><Node NodeType="NodeList" epx="Sequence" ColNo="0" LineNo="16"><NodeId>TestUnknown</NodeId><VariableDeclarations><DeclareVariable ColNo="2" LineNo="18"><Name>r</Name><Type>Real</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="18"><Name>r1</Name><Type>Real</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="18"><Name>r2</Name><Type>Real</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="19"><Name>i</Name><Type>Integer</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="19"><Name>i1</Name><Type>Integer</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="19"><Name>i2</Name><Type>Integer</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="20"><Name>s</Name><Type>String</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="20"><Name>s1</Name><Type>String</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="20"><Name>s2</Name><Type>String</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="21"><Name>b</Name><Type>Boolean</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="21"><Name>b1</Name><Type>Boolean</Type></DeclareVariable><DeclareVariable ColNo="2" LineNo="21"><Name>b2</Name><Type>Boolean</Type></DeclareVariable></VariableDeclarations><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="NodeList" epx="Concurrence" ColNo="2" LineNo="23"><NodeId>TestVars1</NodeId><PostCondition><AND><IsKnown><RealVariable>r1</RealVariable></IsKnown><EQNumeric><RealVariable>r1</RealVariable><RealValue>0.0</RealValue></EQNumeric><IsKnown><IntegerVariable>i1</IntegerVariable></IsKnown><EQNumeric><IntegerVariable>i1</IntegerVariable><IntegerValue>0</IntegerValue></EQNumeric><IsKnown><StringVariable>s1</StringVariable></IsKnown><EQString><StringVariable>s1</StringVariable><StringValue>foo</StringValue></EQString><IsKnown><BooleanVariable>b1</BooleanVariable></IsKnown><BooleanVariable>b1</BooleanVariable><NOT><IsKnown><RealVariable>r2</RealVariable></IsKnown></NOT><NOT><IsKnown><IntegerVariable>i2</IntegerVariable></IsKnown></NOT><NOT><IsKnown><StringVariable>s2</StringVariable></IsKnown></NOT><NOT><IsKnown><BooleanVariable>b2</BooleanVariable></IsKnown></NOT></AND></PostCondition><NodeBody><NodeList><Node NodeType="Assignment" ColNo="4" LineNo="29"><NodeId>ASSIGNMENT__0</NodeId><NodeBody><Assignment ColNo="4" LineNo="29"><RealVariable>r1</RealVariable><NumericRHS><LookupNow><Name><StringValue>r1</StringValue></Name></LookupNow></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="31"><NodeId>ASSIGNMENT__1</NodeId><NodeBody><Assignment ColNo="4" LineNo="31"><RealVariable>r2</RealVariable><NumericRHS><LookupNow><Name><StringValue>r2</StringValue></Name></LookupNow></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="33"><NodeId>ASSIGNMENT__2</NodeId><NodeBody><Assignment ColNo="4" LineNo="33"><IntegerVariable>i1</IntegerVariable><NumericRHS><LookupNow><Name><StringValue>i1</StringValue></Name></LookupNow></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="35"><NodeId>ASSIGNMENT__3</NodeId><NodeBody><Assignment ColNo="4" LineNo="35"><IntegerVariable>i2</IntegerVariable><NumericRHS><LookupNow><Name><StringValue>i2</StringValue></Name></LookupNow></NumericRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="37"><NodeId>ASSIGNMENT__4</NodeId><NodeBody><Assignment ColNo="4" LineNo="37"><StringVariable>s1</StringVariable><StringRHS><LookupNow><Name><StringValue>s1</StringValue></Name></LookupNow></StringRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="39"><NodeId>ASSIGNMENT__5</NodeId><NodeBody><Assignment ColNo="4" LineNo="39"><StringVariable>s2</StringVariable><StringRHS><LookupNow><Name><StringValue>s2</StringValue></Name></LookupNow></StringRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="41"><NodeId>ASSIGNMENT__6</NodeId><NodeBody><Assignment ColNo="4" LineNo="41"><BooleanVariable>b1</BooleanVariable><BooleanRHS><LookupNow><Name><StringValue>b1</StringValue></Name></LookupNow></BooleanRHS></Assignment></NodeBody></Node><Node NodeType="Assignment" ColNo="4" LineNo="43"><NodeId>ASSIGNMENT__7</NodeId><NodeBody><Assignment ColNo="4" LineNo="43"><BooleanVariable>b2</BooleanVariable><BooleanRHS><LookupNow><Name><StringValue>b2</StringValue></Name></LookupNow></BooleanRHS></Assignment></NodeBody></Node></NodeList></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="47"><NodeId>TestString1</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestVars1</NodeRef></Finished></StartCondition><PostCondition><AND><IsKnown><StringVariable>s</StringVariable></IsKnown><EQString><StringVariable>s</StringVariable><StringValue>yes</StringValue></EQString></AND></PostCondition><NodeBody><Command ColNo="4" LineNo="49"><StringVariable>s</StringVariable><Name><StringValue>get_string</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="52"><NodeId>TestString2</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestString1</NodeRef></Finished></StartCondition><PostCondition><NOT><IsKnown><StringVariable>s</StringVariable></IsKnown></NOT></PostCondition><NodeBody><Command ColNo="4" LineNo="54"><StringVariable>s</StringVariable><Name><StringValue>get_string</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="57"><NodeId>TestInt1</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestString2</NodeRef></Finished></StartCondition><PostCondition><AND><IsKnown><IntegerVariable>i</IntegerVariable></IsKnown><EQNumeric><IntegerVariable>i</IntegerVariable><IntegerValue>1</IntegerValue></EQNumeric></AND></PostCondition><NodeBody><Command ColNo="4" LineNo="59"><IntegerVariable>i</IntegerVariable><Name><StringValue>get_int</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="63"><NodeId>TestInt2</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestInt1</NodeRef></Finished></StartCondition><PostCondition><NOT><IsKnown><IntegerVariable>i</IntegerVariable></IsKnown></NOT></PostCondition><NodeBody><Command ColNo="4" LineNo="65"><IntegerVariable>i</IntegerVariable><Name><StringValue>get_int</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="68"><NodeId>TestReal1</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestInt2</NodeRef></Finished></StartCondition><PostCondition><AND><IsKnown><RealVariable>r</RealVariable></IsKnown><EQNumeric><RealVariable>r</RealVariable><RealValue>1.0</RealValue></EQNumeric></AND></PostCondition><NodeBody><Command ColNo="4" LineNo="70"><RealVariable>r</RealVariable><Name><StringValue>get_real</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="74"><NodeId>TestReal2</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestReal1</NodeRef></Finished></StartCondition><PostCondition><NOT><IsKnown><RealVariable>r</RealVariable></IsKnown></NOT></PostCondition><NodeBody><Command ColNo="4" LineNo="76"><RealVariable>r</RealVariable><Name><StringValue>get_real</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="79"><NodeId>TestBool1</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestReal2</NodeRef></Finished></StartCondition><PostCondition><AND><IsKnown><BooleanVariable>b</BooleanVariable></IsKnown><NOT><BooleanVariable>b</BooleanVariable></NOT></AND></PostCondition><NodeBody><Command ColNo="4" LineNo="81"><BooleanVariable>b</BooleanVariable><Name><StringValue>get_bool</StringValue></Name></Command></NodeBody></Node><Node NodeType="Command" ColNo="2" LineNo="85"><NodeId>TestBool2</NodeId><StartCondition><Finished><NodeRef dir="sibling">TestBool1</NodeRef></Finished></StartCondition><PostCondition><NOT><IsKnown><BooleanVariable>b</BooleanVariable></IsKnown></NOT></PostCondition><NodeBody><Command ColNo="4" LineNo="87"><BooleanVariable>b</BooleanVariable><Name><StringValue>get_bool</StringValue></Name></Command></NodeBody></Node></NodeList></NodeBody></Node></PlexilPlan>