<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList" epx="Sequence" FileName="moveLeft.ple" LineNo="2" ColNo="1">
      <NodeId>MoveLeftCommand</NodeId>
      <VariableDeclarations>
              <DeclareVariable>
                  <Name>theTime</Name>
                  <Type>Real</Type>
              </DeclareVariable>
          </VariableDeclarations>
      <InvariantCondition>
         <AND>
            <NOT>
               <OR>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>MoveLeftCommand__CHILD__1</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
               </OR>
            </NOT>
         </AND>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command" FileName="moveLeft.ple" LineNo="5" ColNo="17">
               <NodeId>MoveLeftCommand__CHILD__1</NodeId>
               <NodeBody>
                  <Command>
                      <Name>
                          <StringValue>SendMessage</StringValue>
                      </Name>
                      <Arguments>
                          <StringValue>MoveLeft</StringValue>
                      </Arguments>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>