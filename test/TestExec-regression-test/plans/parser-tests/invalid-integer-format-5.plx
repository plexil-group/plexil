<?xml version="1.0" encoding="UTF-8"?>
<!-- Test cases for detection of bad Integer constant formats -->
<PlexilPlan>
  <Node NodeType="Empty">
    <NodeId>invalid-integer-format-5</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>hex</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0xAF</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
  </Node>
</PlexilPlan>

