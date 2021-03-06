<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList">
    <NodeId>var-priority-with-skip</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>n</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>flag</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>false</BooleanValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareMutex>
        <Name>m</Name>
      </DeclareMutex>
    </VariableDeclarations>
    <PostCondition>
      <EQInternal>
        <NodeOutcomeVariable><NodeId>set-to-2</NodeId></NodeOutcomeVariable>
        <NodeOutcomeValue>SKIPPED</NodeOutcomeValue>
      </EQInternal>
    </PostCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Assignment">
          <NodeId>set-to-1</NodeId>
          <Priority>0</Priority>
          <UsingMutex>
            <Name>m</Name>
          </UsingMutex>
          <NodeBody>
            <Assignment>
              <IntegerVariable>n</IntegerVariable>
              <NumericRHS>
                <IntegerValue>1</IntegerValue>
              </NumericRHS>
            </Assignment>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment">
          <NodeId>set-to-2</NodeId>
          <Priority>1</Priority>
          <UsingMutex>
            <Name>m</Name>
          </UsingMutex>
          <SkipCondition>
            <BooleanVariable>flag</BooleanVariable>
          </SkipCondition>
          <NodeBody>
            <Assignment>
              <IntegerVariable>n</IntegerVariable>
              <NumericRHS>
                <IntegerValue>2</IntegerValue>
              </NumericRHS>
            </Assignment>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment">
          <NodeId>screw-the-pooch</NodeId>
          <NodeBody>
            <Assignment>
              <BooleanVariable>flag</BooleanVariable>
              <BooleanRHS>
                <BooleanValue>true</BooleanValue>
              </BooleanRHS>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
