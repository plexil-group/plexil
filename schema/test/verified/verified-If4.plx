<?xml version="1.0" encoding="UTF-8"?>
<!-- Tests Unknown result -->
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
        <Node NodeType="Empty" epx="Else">
          <StartCondition>
            <NOT>
              <BooleanVariable>test</BooleanVariable>
            </NOT>
          </StartCondition>
          <SkipCondition>
            <BooleanVariable>test</BooleanVariable>
          </SkipCondition>
          <NodeId>Two</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
