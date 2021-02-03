<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <StateDeclaration>
      <Name>X</Name>
      <Return>
        <Name>_return_0</Name>
        <Type>Integer</Type>
      </Return>
    </StateDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>test</Name>
        <Type>Boolean</Type>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="Then">
          <StartCondition>
            <EQNumeric>
              <IntegerValue>2</IntegerValue>
              <LookupNow epx="Lookup">
                <Name>
                  <StringValue>X</StringValue>
                </Name>
              </LookupNow>
            </EQNumeric>
          </StartCondition>
          <SkipCondition>
            <NOT>
              <EQNumeric>
                <IntegerValue>2</IntegerValue>
                <LookupNow epx="Lookup">
                  <Name>
                    <StringValue>X</StringValue>
                  </Name>
                </LookupNow>
              </EQNumeric>
            </NOT>
          </SkipCondition>
          <NodeId generated="1">ep2cp_Sequence_d13e50</NodeId>
          <InvariantCondition>
            <NoChildFailed>
              <NodeRef dir="self"/>
            </NoChildFailed>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Empty">
                <NodeId>A</NodeId>
              </Node>
              <Node NodeType="Assignment">
                <NodeId>B</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">A</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Assignment>
                    <BooleanVariable>test</BooleanVariable>
                    <BooleanRHS>
                      <BooleanValue>true</BooleanValue>
                    </BooleanRHS>
                  </Assignment>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
