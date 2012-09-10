<?xml version="1.0" encoding="UTF-8"?>
<!-- Test cases for detection of bad Integer constant formats -->
<PlexilPlan>
  <Node NodeType="Assignment">
    <NodeId>invalid-integer-format-6</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>hex</Name>
        <Type>Integer</Type>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <Assignment>
        <IntegerVariable>hex</IntegerVariable>
        <NumericRHS>
          <IntegerValue>0xAF</IntegerValue>
        </NumericRHS>
      </Assignment>
    </NodeBody>
  </Node>
</PlexilPlan>

