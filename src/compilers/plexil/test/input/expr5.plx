<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="Assignment" LineNo="5" ColNo="2">
      <NodeId>BLOCK__0</NodeId>
      <VariableDeclarations>
         <DeclareVariable LineNo="2" ColNo="2">
            <Name>i</Name>
            <Type>Integer</Type>
         </DeclareVariable>
      </VariableDeclarations>
      <PostCondition>
         <EQNumeric>
            <IntegerVariable>i</IntegerVariable>
            <IntegerValue>3</IntegerValue>
         </EQNumeric>
      </PostCondition>
      <NodeBody>
         <Assignment>
            <IntegerVariable>i</IntegerVariable>
            <NumericRHS>
               <RealValue>3.2</RealValue>
            </NumericRHS>
         </Assignment>
      </NodeBody>
   </Node>
</PlexilPlan>