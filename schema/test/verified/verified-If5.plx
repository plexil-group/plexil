<?xml version="1.0" encoding="UTF-8"?>
<!-- Tests Unknown result, when there's no Else clause -->
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="If">
    <NodeId generated="1">ep2cp_If_d13e4</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>test</Name>
        <Type>Boolean</Type>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Then">
          <StartCondition>
            <BooleanVariable>test</BooleanVariable>
          </StartCondition>
          <SkipCondition>
            <NOT>
              <BooleanVariable>test</BooleanVariable>
            </NOT>
          </SkipCondition>
          <NodeId>One</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
