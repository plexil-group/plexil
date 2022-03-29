<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
   <GlobalDeclarations>
      <CommandDeclaration>
         <Name>pprint</Name>
         <AnyParameters/>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="CheckedSequence">
      <NodeId generated="1">ep2cp_OnMessage_d13e13</NodeId>
      <VariableDeclarations>
         <DeclareVariable>
            <Name>ep2cp_hdl</Name>
            <Type>String</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <InvariantCondition>
         <NoChildFailed>
            <NodeRef dir="self"/>
         </NoChildFailed>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command" epx="aux">
               <NodeId generated="1">ep2cp_OnMessage_MsgWait</NodeId>
               <EndCondition>
                  <IsKnown>
                     <StringVariable>ep2cp_hdl</StringVariable>
                  </IsKnown>
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
                     <StringVariable>ep2cp_hdl</StringVariable>
                     <Name>
                        <StringValue>ReceiveMessage</StringValue>
                     </Name>
                     <Arguments>
                        <StringValue>moveRover</StringValue>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Command">
               <NodeId generated="1">ep2cp_OnMessage_body</NodeId>
               <StartCondition>
                  <Finished>
                     <NodeRef dir="sibling">ep2cp_OnMessage_MsgWait</NodeRef>
                  </Finished>
               </StartCondition>
               <NodeBody>
                  <Command>
                     <Name>
                        <StringValue>pprint</StringValue>
                     </Name>
                     <Arguments>
                        <StringValue>We gave at the office</StringValue>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
