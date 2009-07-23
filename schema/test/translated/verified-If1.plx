<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="If">
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
            <Node NodeType="Assignment">
               <NodeId>ep2cp_IfSetup</NodeId>
               <NodeBody>
                  <Assignment>
                     <BooleanVariable>ep2cp_test</BooleanVariable>
                     <BooleanRHS>
                        <BooleanVariable>true</BooleanVariable>
                     </BooleanRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="NodeList">
               <NodeId>ep2cp_IfBody</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeId>ep2cp_IfSetup</NodeId>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
               <EndCondition>
                  <OR>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>ep2cp_IfTrueCase</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>ep2cp_IfFalseCase</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </OR>
               </EndCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="NodeList">
                        <NodeId>ep2cp_IfTrueCase</NodeId>
                        <StartCondition>
                           <BooleanVariable>ep2cp_test</BooleanVariable>
                        </StartCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Empty">
                                 <NodeId>One</NodeId>
                              </Node>
                           </NodeList>
                        </NodeBody>
                     </Node>
                     <Node NodeType="NodeList">
                        <NodeId>ep2cp_IfFalseCase</NodeId>
                        <StartCondition>
                           <NOT>
                              <BooleanVariable>ep2cp_test</BooleanVariable>
                           </NOT>
                        </StartCondition>
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