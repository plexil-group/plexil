<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="Concurrence" LineNo="2" ColNo="0">
      <NodeId>InactiveAncestorInvariantTest</NodeId>
      <InvariantCondition>
         <NEInternal>
            <NodeStateVariable>
               <NodeId>TheParent</NodeId>
            </NodeStateVariable>
            <NodeStateValue>EXECUTING</NodeStateValue>
         </NEInternal>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="Sequence" LineNo="9" ColNo="2">
               <NodeId>TheParent</NodeId>
               <InvariantCondition>
                  <AND>
                     <NOT>
                        <OR>
                           <EQInternal>
                              <NodeOutcomeVariable>
                                 <NodeId>TheChild</NodeId>
                              </NodeOutcomeVariable>
                              <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                           </EQInternal>
                        </OR>
                     </NOT>
                  </AND>
               </InvariantCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Empty" LineNo="0" ColNo="0">
                        <NodeId>TheChild</NodeId>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>