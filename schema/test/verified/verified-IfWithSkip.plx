<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <Node NodeType="NodeList" epx="If">
      <NodeId>IfWithSkip</NodeId>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>foo</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>true</BooleanValue>
            </InitialValue>
         </DeclareVariable>
         <DeclareVariable>
            <Name>bar</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>false</BooleanValue>
            </InitialValue>
         </DeclareVariable>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="Empty" epx="Then">
               <StartCondition>
                  <BooleanVariable>foo</BooleanVariable>
               </StartCondition>
               <SkipCondition>
                  <OR>
                     <NOT>
                        <BooleanVariable>foo</BooleanVariable>
                     </NOT>
                     <BooleanVariable>bar</BooleanVariable>
                  </OR>
               </SkipCondition>
               <NodeId>One</NodeId>
            </Node>
            <Node NodeType="Empty" epx="Else">
               <StartCondition>
                  <NOT>
                     <BooleanVariable>foo</BooleanVariable>
                  </NOT>
               </StartCondition>
               <SkipCondition>
                  <OR>
                     <BooleanVariable>foo</BooleanVariable>
                     <NOT>
                        <BooleanVariable>bar</BooleanVariable>
                     </NOT>
                  </OR>
               </SkipCondition>
               <NodeId>Two</NodeId>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
