<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="array-assignment-string.ple">
   <GlobalDeclarations LineNo="1" ColNo="0">
      <CommandDeclaration LineNo="1" ColNo="0">
         <Name>pprint</Name>
         <AnyParameters/>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Sequence" LineNo="2" ColNo="0">
      <NodeId>root</NodeId>
      <VariableDeclarations>
         <DeclareArray LineNo="3" ColNo="2">
            <Name>xs</Name>
            <Type>String</Type>
            <MaxSize>4</MaxSize>
            <InitialValue>
               <ArrayValue Type="String">
                  <StringValue>one</StringValue>
                  <StringValue>two</StringValue>
                  <StringValue>three</StringValue>
                  <StringValue>four</StringValue>
               </ArrayValue>
            </InitialValue>
         </DeclareArray>
         <DeclareArray LineNo="4" ColNo="2">
            <Name>ys</Name>
            <Type>String</Type>
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
            <Node NodeType="Assignment" LineNo="5" ColNo="2">
               <NodeId>plexilisp_Name_1</NodeId>
               <NodeBody>
                  <Assignment LineNo="6" ColNo="4">
                     <ArrayVariable>ys</ArrayVariable>
                     <ArrayRHS>
                        <ArrayVariable>xs</ArrayVariable>
                     </ArrayRHS>
                  </Assignment>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="8" ColNo="2">
               <NodeId>plexilisp_Name_2</NodeId>
               <StartCondition>
                  <AND>
                     <Finished>
                        <NodeRef dir="sibling">plexilisp_Name_1</NodeRef>
                     </Finished>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>plexilisp_Name_1</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <Command LineNo="10" ColNo="4">
                     <Name>
                        <StringValue>pprint</StringValue>
                     </Name>
                     <Arguments>
                        <ArrayElement>
                           <ArrayVariable>ys</ArrayVariable>
                           <Index>
                              <IntegerValue>0</IntegerValue>
                           </Index>
                        </ArrayElement>
                        <ArrayElement>
                           <ArrayVariable>ys</ArrayVariable>
                           <Index>
                              <IntegerValue>1</IntegerValue>
                           </Index>
                        </ArrayElement>
                        <ArrayElement>
                           <ArrayVariable>ys</ArrayVariable>
                           <Index>
                              <IntegerValue>2</IntegerValue>
                           </Index>
                        </ArrayElement>
                        <ArrayElement>
                           <ArrayVariable>ys</ArrayVariable>
                           <Index>
                              <IntegerValue>3</IntegerValue>
                           </Index>
                        </ArrayElement>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
