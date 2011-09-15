<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations LineNo="1" ColNo="0">
        <CommandDeclaration LineNo="1" ColNo="0">
            <Name>A</Name>
        </CommandDeclaration>
        <CommandDeclaration LineNo="2" ColNo="0">
            <Name>B</Name>
            <Parameter>
                <Type>Integer</Type>
            </Parameter>
        </CommandDeclaration>
        <CommandDeclaration LineNo="3" ColNo="0">
            <Name>C</Name>
            <Parameter>
                <Type>Real</Type>
            </Parameter>
        </CommandDeclaration>
    </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Sequence" LineNo="5" ColNo="0">
      <NodeId>Sequence__0</NodeId>
      <InvariantCondition>
         <AND>
            <NOT>
               <OR>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__1</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__2</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__3</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>COMMAND__4</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
               </OR>
            </NOT>
         </AND>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command" LineNo="7" ColNo="2">
               <NodeId>COMMAND__1</NodeId>
               <NodeBody>
                  <Command>
                    <Name>
                        <StringValue>A</StringValue>
                    </Name>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="8" ColNo="2">
               <NodeId>COMMAND__2</NodeId>
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
                  <Command>
                    <Name>
                        <StringValue>B</StringValue>
                    </Name>
                    <Arguments LineNo="8" ColNo="5">
                        <IntegerValue>0</IntegerValue>
                    </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="9" ColNo="2">
               <NodeId>COMMAND__3</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>COMMAND__2</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <Command>
                    <Name>
                        <StringValue>C</StringValue>
                    </Name>
                    <Arguments LineNo="9" ColNo="5">
                        <RealValue>2.3</RealValue>
                    </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="Command" LineNo="10" ColNo="2">
               <NodeId>COMMAND__4</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>COMMAND__3</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <Command>
                    <Name>
                        <StringValue>A</StringValue>
                    </Name>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>