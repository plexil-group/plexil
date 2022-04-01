<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
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
            <Name>
               <StringValue>foo</StringValue>
            </Name>
            <Arguments>
               <StringValue>this</StringValue>
            </Arguments>
         </Command>
      </NodeBody>
   </Node>
</PlexilPlan>
