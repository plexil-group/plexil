<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <GlobalDeclarations>
      <DeclareMutex>
         <Name>m</Name>
      </DeclareMutex>
   </GlobalDeclarations>
   <Node NodeType="NodeList"
          epx="SynchronousCommand_wrapper"
          FileName="foo.ple"
          LineNo="104"
          ColNo="1">
      <NodeId>A</NodeId>
      <UsingMutex>
         <Name>m</Name>
      </UsingMutex>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>x</Name>
            <Type>Integer</Type>
         </DeclareVariable>
         <DeclareVariable>
            <Name>ep2cp_SynchronousCommand_temp</Name>
            <Type>Integer</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command" epx="SynchronousCommandCommand">
               <NodeId>ep2cp_SynchronousCommand_cmd</NodeId>
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
                     <IntegerVariable>ep2cp_SynchronousCommand_temp</IntegerVariable>
                     <Name>
                        <StringValue>foo</StringValue>
                     </Name>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Assignment" epx="SynchronousCommandAssignment">
               <NodeId>ep2cp_SynchronousCommand_assign</NodeId>
               <StartCondition>
                  <IsKnown>
                     <IntegerVariable>ep2cp_SynchronousCommand_temp</IntegerVariable>
                  </IsKnown>
               </StartCondition>
               <NodeBody>
                  <Assignment>
                     <IntegerVariable>x</IntegerVariable>
                     <NumericRHS>
                        <IntegerVariable>ep2cp_SynchronousCommand_temp</IntegerVariable>
                     </NumericRHS>
                  </Assignment>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
