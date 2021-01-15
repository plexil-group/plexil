<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <StateDeclaration>
      <Name>flag</Name>
      <Return>
        <Name>_return_0</Name>
        <Type>Boolean</Type>
      </Return>
    </StateDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>Root</NodeId>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Then">
          <StartCondition>
            <LookupNow epx="Lookup">
              <Name>
                <StringValue>flag</StringValue>
              </Name>
            </LookupNow>
          </StartCondition>
          <SkipCondition>
            <NOT>
              <LookupNow epx="Lookup">
                <Name>
                  <StringValue>flag</StringValue>
                </Name>
              </LookupNow>
            </NOT>
          </SkipCondition>
          <NodeId>A</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
