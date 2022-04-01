<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <Node NodeType="NodeList" epx="Sequence">
      <NodeId generated="1">ep2cp_Sequence_d13e3</NodeId>
      <VariableDeclarations>
      <!-- This is the array used for command FOO: -->
         <DeclareArray>
            <Name>x</Name>
            <Type>Integer</Type>
            <MaxSize>4</MaxSize>
         </DeclareArray>
      </VariableDeclarations>
      <InvariantCondition>
         <NoChildFailed>
            <NodeRef dir="self"/>
         </NoChildFailed>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList"
                  epx="SynchronousCommand_wrapper"
                  FileName="foo.ple"
                  LineNo="104"
                  ColNo="1">
               <NodeId>A</NodeId>
               <VariableDeclarations>
                  <DeclareArray>
                     <Name>ep2cp_SynchronousCommand_temp</Name>
                     <Type>Integer</Type>
                     <MaxSize>4</MaxSize>
                  </DeclareArray>
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
                              <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                              <Name>
                                 <StringValue>FOO</StringValue>
                              </Name>
                           </Command>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" epx="SynchronousCommandAssignment">
                        <NodeId>ep2cp_SynchronousCommand_assign</NodeId>
                        <StartCondition>
                           <IsKnown>
                              <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                           </IsKnown>
                        </StartCondition>
                        <NodeBody>
                           <Assignment>
                              <ArrayVariable>x</ArrayVariable>
                              <ArrayRHS>
                                 <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                              </ArrayRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
            <Node NodeType="NodeList" epx="SynchronousCommand_wrapper">
               <NodeId>B</NodeId>
               <VariableDeclarations>
                  <DeclareArray>
                     <Name>x</Name>
                     <Type>String</Type>
                     <MaxSize>20</MaxSize>
                  </DeclareArray>
                  <DeclareArray>
                     <Name>ep2cp_SynchronousCommand_temp</Name>
                     <Type>String</Type>
                     <MaxSize>20</MaxSize>
                  </DeclareArray>
               </VariableDeclarations>
               <StartCondition>
                  <Finished>
                     <NodeRef dir="sibling">A</NodeRef>
                  </Finished>
               </StartCondition>
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
                              <ResourceList/>
                              <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                              <Name>
                                 <StringValue>BAR</StringValue>
                              </Name>
                           </Command>
                        </NodeBody>
                     </Node>
                     <Node NodeType="Assignment" epx="SynchronousCommandAssignment">
                        <NodeId>ep2cp_SynchronousCommand_assign</NodeId>
                        <StartCondition>
                           <IsKnown>
                              <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                           </IsKnown>
                        </StartCondition>
                        <NodeBody>
                           <Assignment>
                              <ArrayVariable>x</ArrayVariable>
                              <ArrayRHS>
                                 <ArrayVariable>ep2cp_SynchronousCommand_temp</ArrayVariable>
                              </ArrayRHS>
                           </Assignment>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
