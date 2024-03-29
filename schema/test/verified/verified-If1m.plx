<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>true</BooleanValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <UsingMutex>
      <Name>m</Name>
    </UsingMutex>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Then">
          <StartCondition>
            <BooleanVariable>foo</BooleanVariable>
          </StartCondition>
          <SkipCondition>
            <NOT>
              <BooleanVariable>foo</BooleanVariable>
            </NOT>
          </SkipCondition>
          <NodeId>One</NodeId>
        </Node>
        <Node NodeType="Empty" epx="Else">
          <StartCondition>
            <NOT>
              <BooleanVariable>foo</BooleanVariable>
            </NOT>
          </StartCondition>
          <SkipCondition>
            <BooleanVariable>foo</BooleanVariable>
          </SkipCondition>
          <NodeId>Two</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
