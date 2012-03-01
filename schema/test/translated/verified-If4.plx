<?xml version="1.0" encoding="UTF-8"?>
<!-- Tests Unknown result --><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="If">
      <NodeId>ep2cp_If_d1e4</NodeId>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>test</Name>
            <Type>Boolean</Type>
         </DeclareVariable>
         <DeclareVariable>
            <Name>ep2cp_test</Name>
            <Type>Boolean</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="Assignment" epx="aux">
               <NodeId>ep2cp_IfSetup</NodeId>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>ep2cp_test</BooleanVariable>
                     <BooleanRHS>
                        <BooleanVariable>test</BooleanVariable>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="NodeList" epx="aux">
               <NodeId>ep2cp_IfBody</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeId>ep2cp_IfSetup</NodeId>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="NodeList" epx="Then">
                        <NodeId>ep2cp_IfThenCase</NodeId>
                        <StartCondition>
                           <BooleanVariable>ep2cp_test</BooleanVariable>
                        </StartCondition>
                        <SkipCondition>
                           <OR>
                              <NOT>
                                 <IsKnown>
                                    <BooleanVariable>ep2cp_test</BooleanVariable>
                                 </IsKnown>
                              </NOT>
                              <NOT>
                                 <BooleanVariable>ep2cp_test</BooleanVariable>
                              </NOT>
                           </OR>
                        </SkipCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Empty">
                                 <NodeId>One</NodeId>
                              </Node>
                           </NodeList>
                        </NodeBody>
                     </Node>
                     <Node NodeType="NodeList" epx="Else">
                        <NodeId>ep2cp_IfElseCase</NodeId>
                        <StartCondition>
                           <OR>
                              <NOT>
                                 <IsKnown>
                                    <BooleanVariable>ep2cp_test</BooleanVariable>
                                 </IsKnown>
                              </NOT>
                              <NOT>
                                 <BooleanVariable>ep2cp_test</BooleanVariable>
                              </NOT>
                           </OR>
                        </StartCondition>
                        <SkipCondition>
                           <BooleanVariable>ep2cp_test</BooleanVariable>
                        </SkipCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Empty">
                                 <NodeId>Two</NodeId>
                              </Node>
                           </NodeList>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>