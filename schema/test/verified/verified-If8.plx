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
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Then">
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
          <NodeId>A</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
