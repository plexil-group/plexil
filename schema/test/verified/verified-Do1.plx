<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="DoWrapper">
    <NodeId>Do1</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>true</BooleanValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node epx="Do" NodeType="Empty">
          <NodeId>ep2cp_Node_d13e24</NodeId>
          <RepeatCondition>
            <BooleanVariable>foo</BooleanVariable>
          </RepeatCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
