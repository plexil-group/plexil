<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator"
            FileName="array4.ple">
   <GlobalDeclarations LineNo="1" ColNo="8">
      <StateDeclaration LineNo="1" ColNo="8">
         <Name>a4</Name>
         <Return>
            <Name>_return_0</Name>
            <Type>Real</Type>
            <MaxSize>4</MaxSize>
         </Return>
      </StateDeclaration>
   </GlobalDeclarations>
   <Node NodeType="Assignment" LineNo="2" ColNo="0">
      <NodeId>array4</NodeId>
      <VariableDeclarations>
         <DeclareArray LineNo="3" ColNo="2">
            <Name>a4</Name>
            <Type>Real</Type>
            <MaxSize>500</MaxSize>
         </DeclareArray>
      </VariableDeclarations>
      <PostCondition>
         <EQNumeric>
            <ArrayElement>
               <ArrayVariable>a4</ArrayVariable>
               <Index>
                  <IntegerValue>1</IntegerValue>
               </Index>
            </ArrayElement>
            <RealValue>0.0</RealValue>
         </EQNumeric>
      </PostCondition>
      <NodeBody>
         <Assignment LineNo="5" ColNo="2">
            <ArrayVariable>a4</ArrayVariable>
            <ArrayRHS>
               <LookupNow>
                  <Name>
                     <StringValue>a4</StringValue>
                  </Name>
               </LookupNow>
            </ArrayRHS>
         </Assignment>
      </NodeBody>
   </Node>
</PlexilPlan>
