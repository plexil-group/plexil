<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="If">
      <NodeId>ep2cp_If_d1e3</NodeId>
      <VariableDeclarations>
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
                        <BooleanValue>true</BooleanValue>
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
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>