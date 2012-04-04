<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations LineNo="1" ColNo="0">
      <LibraryNodeDeclaration LineNo="1" ColNo="0">
         <Name>Increment</Name>
         <Interface LineNo="1" ColNo="25">
            <In>
               <DeclareVariable LineNo="1" ColNo="25">
                  <Name>x</Name>
                  <Type>Integer</Type>
               </DeclareVariable>
            </In>
            <InOut>
               <DeclareVariable LineNo="1" ColNo="39">
                  <Name>result</Name>
                  <Type>Integer</Type>
               </DeclareVariable>
            </InOut>
         </Interface>
      </LibraryNodeDeclaration>
      <CommandDeclaration LineNo="2" ColNo="0">
         <Name>pprint</Name>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Sequence" LineNo="6" ColNo="2">
      <NodeId>LibraryCallTest</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="6" ColNo="2">
            <Name>result</Name>
            <Type>Integer</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <InvariantCondition>
         <AND>
            <NOT>
               <OR>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>LibraryCall__0</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__1</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>LibraryCall__2</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__3</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
               </OR>
            </NOT>
         </AND>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="LibraryNodeCall">
               <NodeId>LibraryCall__0</NodeId>
               <NodeBody>
                  <LibraryNodeCall>
                     <NodeId>Increment</NodeId>
                     <Alias>
                        <NodeParameter>x</NodeParameter>
                        <IntegerValue>1</IntegerValue>
                     </Alias>
                     <Alias>
                        <NodeParameter>result</NodeParameter>
                        <IntegerVariable>result</IntegerVariable>
                     </Alias>
                  </LibraryNodeCall>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="8" ColNo="2">
               <NodeId>COMMAND__1</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>LibraryCall__0</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <Command>
                     <Name>
                        <StringValue>pprint</StringValue>
                     </Name>
                     <Arguments LineNo="8" ColNo="10">
                        <StringValue>Increment(1) =</StringValue>
                        <IntegerVariable>result</IntegerVariable>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="LibraryNodeCall">
               <NodeId>LibraryCall__2</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>COMMAND__1</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <LibraryNodeCall>
                     <NodeId>Increment</NodeId>
                     <Alias>
                        <NodeParameter>x</NodeParameter>
                        <IntegerVariable>result</IntegerVariable>
                     </Alias>
                     <Alias>
                        <NodeParameter>result</NodeParameter>
                        <IntegerVariable>result</IntegerVariable>
                     </Alias>
                  </LibraryNodeCall>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="10" ColNo="2">
               <NodeId>COMMAND__3</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>LibraryCall__2</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <Command>
                     <Name>
                        <StringValue>pprint</StringValue>
                     </Name>
                     <Arguments LineNo="10" ColNo="10">
                        <StringValue>Increment(2) =</StringValue>
                        <IntegerVariable>result</IntegerVariable>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>