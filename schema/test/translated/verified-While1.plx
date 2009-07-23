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
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList">
               <NodeId>ep2cp_WhileBody</NodeId>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="NodeList">
                        <NodeId>ep2cp_WhileTrue</NodeId>
                        <StartCondition>
                           <BooleanVariable>foo</BooleanVariable>
                        </StartCondition>
                        <RepeatCondition>
                           <BooleanVariable>foo</BooleanVariable>
                        </RepeatCondition>
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