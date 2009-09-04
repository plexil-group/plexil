<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="While">
      <NodeId>ep2cp_While_d1e3</NodeId>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>ep2cp_test</Name>
            <Type>Boolean</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="aux">
               <NodeId>ep2cp_WhileBody</NodeId>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Assignment" epx="aux">
                        <NodeId>ep2cp_WhileSetup</NodeId>
                        <NodeBody>
                           <Assignment>
                              <BooleanVariable>ep2cp_test</BooleanVariable>
                              <BooleanRHS>
                                 <LT>
                                    <IntegerValue>0</IntegerValue>
                                    <IntegerValue>1</IntegerValue>
                                 </LT>
                              </BooleanRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                     <Node NodeType="NodeList" epx="aux">
                        <NodeId>ep2cp_WhileTrue</NodeId>
                        <StartCondition>
                           <AND>
                              <EQInternal>
                                 <NodeStateVariable>
                                    <NodeId>ep2cp_WhileSetup</NodeId>
                                 </NodeStateVariable>
                                 <NodeStateValue>FINISHED</NodeStateValue>
                              </EQInternal>
                              <BooleanVariable>ep2cp_test</BooleanVariable>
                           </AND>
                        </StartCondition>
                        <SkipCondition>
                           <NOT>
                              <BooleanVariable>ep2cp_test</BooleanVariable>
                           </NOT>
                        </SkipCondition>
                        <RepeatCondition>
                           <BooleanVariable>ep2cp_test</BooleanVariable>
                        </RepeatCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="NodeList" epx="aux">
                                 <NodeId>ep2cp_WhileAction</NodeId>
                                 <NodeBody>
                                    <NodeList>
                                       <Node NodeType="Empty">
                                          <NodeId>One</NodeId>
                                       </Node>
                                    </NodeList>
                                 </NodeBody>
                              </Node>
                              <Node NodeType="Assignment" epx="aux">
                                 <NodeId>ep2cp_WhileRetest</NodeId>
                                 <StartCondition>
                                    <EQInternal>
                                       <NodeStateVariable>
                                          <NodeId>ep2cp_WhileAction</NodeId>
                                       </NodeStateVariable>
                                       <NodeStateValue>FINISHED</NodeStateValue>
                                    </EQInternal>
                                 </StartCondition>
                                 <NodeBody>
                                    <Assignment>
                                       <BooleanVariable>ep2cp_test</BooleanVariable>
                                       <BooleanRHS>
                                          <LT>
                                             <IntegerValue>0</IntegerValue>
                                             <IntegerValue>1</IntegerValue>
                                          </LT>
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
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>