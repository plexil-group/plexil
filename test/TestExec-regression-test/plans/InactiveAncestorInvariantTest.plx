<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="InactiveAncestorInvariantTest.ple">
   <Node NodeType="NodeList" epx="Concurrence" LineNo="3" ColNo="0">
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
            <Node NodeType="NodeList" epx="Sequence" LineNo="10" ColNo="2">
               <NodeId>TheParent</NodeId>
               <InvariantCondition>
                  <NOT>
                     <AND>
                        <EQInternal>
                           <NodeOutcomeVariable>
                              <NodeRef dir="child">TheChild</NodeRef>
                           </NodeOutcomeVariable>
                           <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                        </EQInternal>
                        <EQInternal>
                           <NodeStateVariable>
                              <NodeRef dir="child">TheChild</NodeRef>
                           </NodeStateVariable>
                           <NodeStateValue>FINISHED</NodeStateValue>
                        </EQInternal>
                     </AND>
                  </NOT>
               </InvariantCondition>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="Empty" LineNo="1" ColNo="0">
                        <NodeId>TheChild</NodeId>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>