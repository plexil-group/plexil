<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="AncestorReferenceTest.ple">
   <Node NodeType="NodeList" epx="Sequence" LineNo="4" ColNo="3">
      <NodeId>AncestorReferenceTest</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="3" ColNo="3">
            <Name>run_start_time</Name>
            <Type>Real</Type>
         </DeclareVariable>
         <DeclareVariable LineNo="3" ColNo="3">
            <Name>r</Name>
            <Type>Real</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <InvariantCondition>
         <NOT>
            <AND>
               <EQInternal>
                  <NodeOutcomeVariable>
                     <NodeRef dir="child">Sub</NodeRef>
                  </NodeOutcomeVariable>
                  <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
               </EQInternal>
               <EQInternal>
                  <NodeStateVariable>
                     <NodeRef dir="child">Sub</NodeRef>
                  </NodeStateVariable>
                  <NodeStateValue>FINISHED</NodeStateValue>
               </EQInternal>
            </AND>
         </NOT>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="Sequence" LineNo="7" ColNo="7">
               <NodeId>Sub</NodeId>
               <InvariantCondition>
                  <NOT>
                     <OR>
                        <AND>
                           <EQInternal>
                              <NodeOutcomeVariable>
                                 <NodeRef dir="child">ASSIGNMENT__0</NodeRef>
                              </NodeOutcomeVariable>
                              <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                           </EQInternal>
                           <EQInternal>
                              <NodeStateVariable>
                                 <NodeRef dir="child">ASSIGNMENT__0</NodeRef>
                              </NodeStateVariable>
                              <NodeStateValue>FINISHED</NodeStateValue>
                           </EQInternal>
                        </AND>
                        <AND>
                           <EQInternal>
                              <NodeOutcomeVariable>
                                 <NodeRef dir="child">ASSIGNMENT__1</NodeRef>
                              </NodeOutcomeVariable>
                              <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                           </EQInternal>
                           <EQInternal>
                              <NodeStateVariable>
                                 <NodeRef dir="child">ASSIGNMENT__1</NodeRef>
                              </NodeStateVariable>
                              <NodeStateValue>FINISHED</NodeStateValue>
                           </EQInternal>
                        </AND>
                     </OR>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" LineNo="7" ColNo="7">
                        <NodeId>ASSIGNMENT__0</NodeId>
                        <NodeBody>
                           <Assignment>
                              <RealVariable>run_start_time</RealVariable>
                              <NumericRHS>
                                 <NodeTimepointValue>
                                    <NodeId>AncestorReferenceTest</NodeId>
                                    <NodeStateValue>EXECUTING</NodeStateValue>
                                    <Timepoint>START</Timepoint>
                                 </NodeTimepointValue>
                              </NumericRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" LineNo="8" ColNo="7">
                        <NodeId>ASSIGNMENT__1</NodeId>
                        <StartCondition>
                           <EQInternal>
                              <NodeStateVariable>
                                 <NodeRef dir="sibling">ASSIGNMENT__0</NodeRef>
                              </NodeStateVariable>
                              <NodeStateValue>FINISHED</NodeStateValue>
                           </EQInternal>
                        </StartCondition>
                        <NodeBody>
                           <Assignment>
                              <RealVariable>r</RealVariable>
                              <NumericRHS>
                                 <RealVariable>run_start_time</RealVariable>
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