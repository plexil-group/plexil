<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="Sequence" FileName="sequence2.ple" LineNo="2"
         ColNo="1">
      <NodeId>Root</NodeId>
      <InvariantCondition>
         <AND>
            <NOT>
               <OR>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>Root__CHILD__1</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
               </OR>
            </NOT>
         </AND>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Empty" FileName="sequence2.ple" LineNo="3" ColNo="12">
               <NodeId>Root__CHILD__1</NodeId>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>