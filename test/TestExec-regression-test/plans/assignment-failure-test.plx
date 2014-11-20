<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="assignment-failure-test.ple">
   <Node NodeType="NodeList" epx="Concurrence" LineNo="3" ColNo="0">
      <NodeId>AssignmentFailureTest</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="4" ColNo="2">
            <Name>spoil</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>false</BooleanValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable LineNo="5" ColNo="2">
            <Name>b</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>true</BooleanValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable LineNo="6" ColNo="2">
            <Name>i</Name>
            <Type>Integer</Type>
            <InitialValue>
               <IntegerValue>1</IntegerValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable LineNo="7" ColNo="2">
            <Name>r</Name>
            <Type>Real</Type>
            <InitialValue>
               <RealValue>1.0</RealValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable LineNo="8" ColNo="2">
            <Name>s</Name>
            <Type>String</Type>
            <InitialValue>
               <StringValue>ess</StringValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareArray LineNo="9" ColNo="2">
            <Name>ra</Name>
            <Type>Real</Type>
            <MaxSize>2</MaxSize>
            <InitialValue>
               <RealValue>1.0</RealValue>
               <RealValue>2.0</RealValue>
            </InitialValue>
         </DeclareArray>
         <DeclareArray LineNo="10" ColNo="2">
            <Name>sa</Name>
            <Type>String</Type>
            <MaxSize>2</MaxSize>
            <InitialValue>
               <StringValue>foo</StringValue>
               <StringValue>bar</StringValue>
            </InitialValue>
         </DeclareArray>
      </VariableDeclarations>
      <EndCondition>
         <BooleanVariable>spoil</BooleanVariable>
      </EndCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Assignment" LineNo="18" ColNo="4">
               <NodeId>BoolFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>b</BooleanVariable>
                     <BooleanRHS>
                        <BooleanValue>false</BooleanValue>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="24" ColNo="4">
               <NodeId>IntFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <IntegerVariable>i</IntegerVariable>
                     <NumericRHS>
                        <IntegerValue>42</IntegerValue>
                     </NumericRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="30" ColNo="4">
               <NodeId>RealFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <RealVariable>r</RealVariable>
                     <NumericRHS>
                        <RealValue>69.0</RealValue>
                     </NumericRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="36" ColNo="4">
               <NodeId>StringFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <StringVariable>s</StringVariable>
                     <StringRHS>
                        <StringValue>S</StringValue>
                     </StringRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="42" ColNo="4">
               <NodeId>ArrayFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <ArrayVariable>ra</ArrayVariable>
                     <ArrayRHS>
                        <ArrayValue Type="Real">
                           <RealValue>42.0</RealValue>
                           <RealValue>69.0</RealValue>
                        </ArrayValue>
                     </ArrayRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="48" ColNo="4">
               <NodeId>StringArrayFail</NodeId>
               <InvariantCondition>
                  <NOT>
                     <BooleanVariable>spoil</BooleanVariable>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <Assignment>
                     <ArrayVariable>sa</ArrayVariable>
                     <ArrayRHS>
                        <ArrayValue Type="String">
                           <StringValue>ha</StringValue>
                           <StringValue>hee</StringValue>
                        </ArrayValue>
                     </ArrayRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="52" ColNo="2">
               <NodeId>theSpoiler</NodeId>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>spoil</BooleanVariable>
                     <BooleanRHS>
                        <BooleanValue>true</BooleanValue>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>