<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="Sequence">
      <NodeId>ep2cp_Sequence_d1e3</NodeId>
      <InvariantCondition>
         <NOT>
            <OR>
               <EQInternal>
                  <NodeOutcomeVariable>
                     <NodeId>One</NodeId>
                  </NodeOutcomeVariable>
                  <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
               </EQInternal>
               <EQInternal>
                  <NodeOutcomeVariable>
                     <NodeId>Two</NodeId>
                  </NodeOutcomeVariable>
                  <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
               </EQInternal>
            </OR>
         </NOT>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Empty">
               <NodeId>One</NodeId>
            </Node>
            <Node NodeType="Empty">
               <NodeId>Two</NodeId>
               <StartCondition>
                  <EQInternal>
                     <NodeStateVariable>
                        <NodeId>One</NodeId>
                     </NodeStateVariable>
                     <NodeStateValue>FINISHED</NodeStateValue>
                  </EQInternal>
               </StartCondition>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>