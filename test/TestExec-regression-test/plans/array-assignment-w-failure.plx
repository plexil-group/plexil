<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="array-assignment-w-failure.ple">
   <Node NodeType="NodeList" epx="Sequence" LineNo="4" ColNo="2">
      <NodeId>ArrayAssignmentWithFailure</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="3" ColNo="2">
            <Name>spoil</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>false</BooleanValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareArray LineNo="4" ColNo="2">
            <Name>ary</Name>
            <Type>Real</Type>
            <MaxSize>3</MaxSize>
            <InitialValue>
               <RealValue>1.0</RealValue>
               <RealValue>2.0</RealValue>
               <RealValue>3.0</RealValue>
            </InitialValue>
         </DeclareArray>
         <DeclareArray LineNo="5" ColNo="2">
            <Name>sa</Name>
            <Type>String</Type>
            <MaxSize>3</MaxSize>
            <InitialValue>
               <StringValue>eh</StringValue>
               <StringValue>bee</StringValue>
               <StringValue>see</StringValue>
            </InitialValue>
         </DeclareArray>
      </VariableDeclarations>
      <PostCondition>
         <AND>
            <BooleanVariable>spoil</BooleanVariable>
            <EQNumeric>
               <ArrayElement>
                  <Name>ary</Name>
                  <Index>
                     <IntegerValue>1</IntegerValue>
                  </Index>
               </ArrayElement>
               <RealValue>42.0</RealValue>
            </EQNumeric>
            <EQString>
               <ArrayElement>
                  <Name>sa</Name>
                  <Index>
                     <IntegerValue>1</IntegerValue>
                  </Index>
               </ArrayElement>
               <StringValue>B</StringValue>
            </EQString>
         </AND>
      </PostCondition>
      <InvariantCondition>
         <NOT>
            <OR>
               <AND>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeRef dir="child">Concurrence__0</NodeRef>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="child">Concurrence__0</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </AND>
               <AND>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeRef dir="child">Concurrence__3</NodeRef>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="child">Concurrence__3</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </AND>
               <AND>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeRef dir="child">ASSIGNMENT__7</NodeRef>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="child">ASSIGNMENT__7</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </AND>
               <AND>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeRef dir="child">Concurrence__8</NodeRef>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="child">Concurrence__8</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </AND>
            </OR>
         </NOT>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="Concurrence" LineNo="12" ColNo="2">
               <NodeId>Concurrence__0</NodeId>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="14" ColNo="4">
                        <NodeId>ASSIGNMENT__1</NodeId>
                        <NodeBody>
                           <Assignment>
                              <ArrayElement>
                                 <Name>ary</Name>
                                 <Index>
                                    <IntegerValue>1</IntegerValue>
                                 </Index>
                              </ArrayElement>
                              <NumericRHS>
                                 <RealValue>42.0</RealValue>
                              </NumericRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="15" ColNo="4">
                        <NodeId>ASSIGNMENT__2</NodeId>
                        <NodeBody>
                           <Assignment>
                              <ArrayElement>
                                 <Name>sa</Name>
                                 <Index>
                                    <IntegerValue>1</IntegerValue>
                                 </Index>
                              </ArrayElement>
                              <StringRHS>
                                 <StringValue>B</StringValue>
                              </StringRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
            <Node NodeType="NodeList" epx="Concurrence" LineNo="19" ColNo="2">
               <NodeId>Concurrence__3</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="sibling">Concurrence__0</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="24" ColNo="6">
                        <NodeId>SpoiledAssignment1</NodeId>
                        <InvariantCondition>
                           <NOT>
                              <BooleanVariable>spoil</BooleanVariable>
                           </NOT>
                        </InvariantCondition>
                        <NodeBody>
                           <Assignment>
                              <ArrayElement>
                                 <Name>ary</Name>
                                 <Index>
                                    <IntegerValue>1</IntegerValue>
                                 </Index>
                              </ArrayElement>
                              <NumericRHS>
                                 <RealValue>69.0</RealValue>
                              </NumericRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="29" ColNo="6">
                        <NodeId>SpoiledAssignment2</NodeId>
                        <InvariantCondition>
                           <NOT>
                              <BooleanVariable>spoil</BooleanVariable>
                           </NOT>
                        </InvariantCondition>
                        <NodeBody>
                           <Assignment>
                              <ArrayElement>
                                 <Name>sa</Name>
                                 <Index>
                                    <IntegerValue>1</IntegerValue>
                                 </Index>
                              </ArrayElement>
                              <StringRHS>
                                 <StringValue>bea</StringValue>
                              </StringRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="31" ColNo="4">
                        <NodeId>ASSIGNMENT__6</NodeId>
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
            <Node NodeType="Assignment" LineNo="34" ColNo="2">
               <NodeId>ASSIGNMENT__7</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="sibling">Concurrence__3</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>spoil</BooleanVariable>
                     <BooleanRHS>
                        <BooleanValue>false</BooleanValue>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="NodeList" epx="Concurrence" LineNo="36" ColNo="2">
               <NodeId>Concurrence__8</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeRef dir="sibling">ASSIGNMENT__7</NodeRef>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="41" ColNo="6">
                        <NodeId>SpoiledAssignment3</NodeId>
                        <InvariantCondition>
                           <NOT>
                              <BooleanVariable>spoil</BooleanVariable>
                           </NOT>
                        </InvariantCondition>
                        <NodeBody>
                           <Assignment>
                              <ArrayVariable>ary</ArrayVariable>
                              <ArrayRHS>
                                 <ArrayValue Type="Real">
                                    <RealValue>4.0</RealValue>
                                    <RealValue>5.0</RealValue>
                                    <RealValue>6.0</RealValue>
                                 </ArrayValue>
                              </ArrayRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="46" ColNo="6">
                        <NodeId>SpoiledAssignment4</NodeId>
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
                                    <StringValue>zee</StringValue>
                                    <StringValue>why</StringValue>
                                    <StringValue>ecks</StringValue>
                                 </ArrayValue>
                              </ArrayRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="48" ColNo="4">
                        <NodeId>ASSIGNMENT__11</NodeId>
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
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>