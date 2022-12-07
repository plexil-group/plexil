<?xml version="1.0" encoding="utf-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="UncheckedSequence">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <UsingMutex>
      <Name>m</Name>
    </UsingMutex>
    <NodeBody>
      <NodeList>
        <Node NodeType="Assignment">
          <NodeId>One</NodeId>
          <PostCondition>
            <EQNumeric>
              <IntegerVariable>foo</IntegerVariable>
              <IntegerValue>3</IntegerValue>
            </EQNumeric>
          </PostCondition>
          <NodeBody>
            <Assignment>
              <IntegerVariable>foo</IntegerVariable>
              <NumericRHS>
                <IntegerValue>3</IntegerValue>
              </NumericRHS>
            </Assignment>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment">
          <NodeId>Two</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">One</NodeRef>
            </Finished>
          </StartCondition>
          <PostCondition>
            <EQNumeric>
              <IntegerVariable>foo</IntegerVariable>
              <IntegerValue>6</IntegerValue>
            </EQNumeric>
          </PostCondition>
          <NodeBody>
            <Assignment>
              <IntegerVariable>foo</IntegerVariable>
              <NumericRHS>
                <IntegerValue>6</IntegerValue>
              </NumericRHS>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
