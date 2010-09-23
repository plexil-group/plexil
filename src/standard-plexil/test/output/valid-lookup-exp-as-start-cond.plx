<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations>
          <StateDeclaration>
              <Name>LAPR02FC0617U</Name>
              <Return Type="Real">_State_return_1</Return>
          </StateDeclaration>
      </GlobalDeclarations>
   <Node NodeType="NodeList" FileName="lookup-exp-as-start-cond.ple" LineNo="4"
         ColNo="1">
      <NodeId>testNode</NodeId>
      <VariableDeclarations>
              <DeclareVariable>
                  <Name>x</Name>
                  <Type>Real</Type>
              </DeclareVariable>
          </VariableDeclarations>
      <StartCondition>
         <EQNumeric>
            <LookupOnChange>
                      <Name>
                          <StringValue>LAPR02FC0617U</StringValue>
                      </Name>
                  </LookupOnChange>
            <RealVariable>x</RealVariable>
         </EQNumeric>
      </StartCondition>
      <NodeBody>
              <NodeList>
                  <Node NodeType="Empty" FileName="lookup-exp-as-start-cond.ple" LineNo="8" ColNo="4">
               <NodeId>testNode__CHILD__1</NodeId>
               <EndCondition>
                  <NENumeric>
                     <LookupOnChange>
                        <Name>
                           <StringValue>LAPR02FC0617U</StringValue>
                        </Name>
                     </LookupOnChange>
                     <RealVariable>x</RealVariable>
                  </NENumeric>
               </EndCondition>
            </Node>
              </NodeList>
          </NodeBody>
   </Node>
</PlexilPlan>