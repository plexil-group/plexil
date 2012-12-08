<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations LineNo="1" ColNo="5">
      <StateDeclaration LineNo="1" ColNo="5">
         <Name>ActualSpeed</Name>
         <Return>
            <Name>_return_0</Name>
            <Type>Real</Type>
         </Return>
      </StateDeclaration>
      <StateDeclaration LineNo="2" ColNo="8">
         <Name>CancelPressed</Name>
         <Return>
            <Name>_return_0</Name>
            <Type>Boolean</Type>
         </Return>
      </StateDeclaration>
      <StateDeclaration LineNo="3" ColNo="8">
         <Name>SetPressed</Name>
         <Return>
            <Name>_return_0</Name>
            <Type>Boolean</Type>
         </Return>
      </StateDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Concurrence" LineNo="5" ColNo="15">
      <NodeId>CruiseControl</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="6" ColNo="2">
            <Name>cruiseOn</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>false</BooleanValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable LineNo="7" ColNo="2">
            <Name>targetSpeed</Name>
            <Type>Real</Type>
         </DeclareVariable>
         <DeclareVariable LineNo="7" ColNo="2">
            <Name>throttlePos</Name>
            <Type>Real</Type>
            <InitialValue>
               <RealValue>0.0</RealValue>
            </InitialValue>
         </DeclareVariable>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="Concurrence" LineNo="9" ColNo="7">
               <NodeId>SET</NodeId>
               <StartCondition>
                  <LookupOnChange>
                     <Name>
                        <StringValue>SetPressed</StringValue>
                     </Name>
                  </LookupOnChange>
               </StartCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="11" ColNo="17">
                        <NodeId>SetCruiseOn</NodeId>
                        <NodeBody>
                           <Assignment>
                              <BooleanVariable>cruiseOn</BooleanVariable>
                              <BooleanRHS>
                                 <BooleanValue>true</BooleanValue>
                              </BooleanRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="12" ColNo="20">
                        <NodeId>SetTargetSpeed</NodeId>
                        <NodeBody>
                           <Assignment>
                              <RealVariable>targetSpeed</RealVariable>
                              <NumericRHS>
                                 <LookupNow>
                                    <Name>
                                       <StringValue>ActualSpeed</StringValue>
                                    </Name>
                                 </LookupNow>
                              </NumericRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" LineNo="18" ColNo="4">
               <NodeId>CANCEL</NodeId>
               <StartCondition>
                  <LookupOnChange>
                     <Name>
                        <StringValue>CancelPressed</StringValue>
                     </Name>
                  </LookupOnChange>
               </StartCondition>
               <RepeatCondition>
                  <BooleanValue>true</BooleanValue>
               </RepeatCondition>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>cruiseOn</BooleanVariable>
                     <BooleanRHS>
                        <BooleanValue>false</BooleanValue>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Empty" LineNo="0" ColNo="0">
               <NodeId>RESUME</NodeId>
            </Node>
            <Node NodeType="Empty" LineNo="0" ColNo="0">
               <NodeId>ACCEL</NodeId>
            </Node>
            <Node NodeType="Empty" LineNo="0" ColNo="0">
               <NodeId>DECEL</NodeId>
            </Node>
            <Node NodeType="NodeList" epx="Concurrence" LineNo="25" ColNo="29">
               <NodeId>ControlSpeedAutomatically</NodeId>
               <StartCondition>
                  <BooleanVariable>cruiseOn</BooleanVariable>
               </StartCondition>
               <RepeatCondition>
                  <BooleanVariable>cruiseOn</BooleanVariable>
               </RepeatCondition>
               <EndCondition>
                  <NOT>
                     <BooleanVariable>cruiseOn</BooleanVariable>
                  </NOT>
               </EndCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="31" ColNo="6">
                        <NodeId>IncreaseThrottle</NodeId>
                        <StartCondition>
                           <LT>
                              <LookupOnChange>
                                 <Name>
                                    <StringValue>ActualSpeed</StringValue>
                                 </Name>
                              </LookupOnChange>
                              <RealVariable>targetSpeed</RealVariable>
                           </LT>
                        </StartCondition>
                        <NodeBody>
                           <Assignment>
                              <RealVariable>throttlePos</RealVariable>
                              <NumericRHS>
                                 <ADD LineNo="31" ColNo="32">
                                    <RealVariable>throttlePos</RealVariable>
                                    <RealValue>0.1</RealValue>
                                 </ADD>
                              </NumericRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="35" ColNo="6">
                        <NodeId>DecreaseThrottle</NodeId>
                        <StartCondition>
                           <GT>
                              <LookupOnChange>
                                 <Name>
                                    <StringValue>ActualSpeed</StringValue>
                                 </Name>
                              </LookupOnChange>
                              <RealVariable>targetSpeed</RealVariable>
                           </GT>
                        </StartCondition>
                        <NodeBody>
                           <Assignment>
                              <RealVariable>throttlePos</RealVariable>
                              <NumericRHS>
                                 <SUB LineNo="35" ColNo="32">
                                    <RealVariable>throttlePos</RealVariable>
                                    <RealValue>0.1</RealValue>
                                 </SUB>
                              </NumericRHS>
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