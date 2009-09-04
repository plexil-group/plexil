<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="While">
      <NodeId>Root</NodeId>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>foo</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>true</BooleanValue>
            </InitialValue>
         </DeclareVariable>
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
                                 <BooleanVariable>foo</BooleanVariable>
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
                                          <BooleanVariable>foo</BooleanVariable>
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