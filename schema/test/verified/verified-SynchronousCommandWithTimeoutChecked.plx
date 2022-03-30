<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <Node NodeType="Command" epx="SynchronousCommand">
      <NodeId>SynchronousCommandWithTimeoutChecked</NodeId>
      <InvariantCondition>
         <LT>
            <LookupOnChange>
               <Name>
                  <StringValue>time</StringValue>
               </Name>
            </LookupOnChange>
            <ADD>
               <RealValue>5</RealValue>
               <NodeTimepointValue>
                  <NodeRef dir="self"/>
                  <NodeStateValue>EXECUTING</NodeStateValue>
                  <Timepoint>START</Timepoint>
               </NodeTimepointValue>
            </ADD>
         </LT>
      </InvariantCondition>
      <EndCondition>
         <EQInternal>
            <NodeCommandHandleVariable>
               <NodeRef dir="self"/>
            </NodeCommandHandleVariable>
            <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
         </EQInternal>
      </EndCondition>
      <PostCondition>
         <EQInternal>
            <NodeCommandHandleVariable>
               <NodeRef dir="self"/>
            </NodeCommandHandleVariable>
            <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
         </EQInternal>
      </PostCondition>
      <NodeBody>
         <Command>
            <Name>
               <StringValue>foo</StringValue>
            </Name>
         </Command>
      </NodeBody>
   </Node>
</PlexilPlan>
