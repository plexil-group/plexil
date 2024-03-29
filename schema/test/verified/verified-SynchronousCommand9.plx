<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <Node NodeType="NodeList" epx="Sequence">
      <NodeId generated="1">ep2cp_Sequence_d14e3</NodeId>
      <InvariantCondition>
         <NoChildFailed>
            <NodeRef dir="self"/>
         </NoChildFailed>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command"
                   epx="SynchronousCommand"
                   FileName="foo.ple"
                   LineNo="104"
                   ColNo="1">
               <NodeId>A</NodeId>
               <EndCondition>
                  <EQInternal>
                     <NodeCommandHandleVariable>
                        <NodeRef dir="self"/>
                     </NodeCommandHandleVariable>
                     <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                  </EQInternal>
               </EndCondition>
               <NodeBody>
                  <Command>
                     <ResourceList>
                        <Resource>
                           <ResourceName>
                              <StringValue>arm</StringValue>
                           </ResourceName>
                           <ResourcePriority>
                              <IntegerValue>3</IntegerValue>
                           </ResourcePriority>
                        </Resource>
                     </ResourceList>
                     <Name>
                        <StringValue>FOO</StringValue>
                     </Name>
                     <Arguments>
                        <StringValue>this</StringValue>
                        <StringValue>that</StringValue>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Command" epx="SynchronousCommand">
               <NodeId>B</NodeId>
               <StartCondition>
                  <Finished>
                     <NodeRef dir="sibling">A</NodeRef>
                  </Finished>
               </StartCondition>
               <EndCondition>
                  <EQInternal>
                     <NodeCommandHandleVariable>
                        <NodeRef dir="self"/>
                     </NodeCommandHandleVariable>
                     <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                  </EQInternal>
               </EndCondition>
               <NodeBody>
                  <Command>
                     <ResourceList/>
                     <Name>
                        <StringValue>BAR</StringValue>
                     </Name>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
