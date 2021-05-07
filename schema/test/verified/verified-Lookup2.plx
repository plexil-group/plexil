<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="Command">
    <NodeId>root</NodeId>
    <StartCondition>
      <LookupOnChange epx="Lookup">
        <Name>
          <StringValue>Foo</StringValue>
        </Name>
        <Tolerance>
          <RealValue>1.0</RealValue>
        </Tolerance>
        <Arguments>
          <RealValue>2.3</RealValue>
        </Arguments>
      </LookupOnChange>
    </StartCondition>
    <InvariantCondition>
      <LookupNow epx="Lookup">
        <Name>
          <StringValue>Bar</StringValue>
        </Name>
        <Arguments>
          <RealValue>3.0</RealValue>
        </Arguments>
      </LookupNow>
    </InvariantCondition>
    <NodeBody>
      <Command>
        <Name>
          <LookupNow epx="Lookup">
            <Name>
              <StringValue>CommandName</StringValue>
            </Name>
          </LookupNow>
        </Name>
        <Arguments>
          <LookupNow epx="Lookup">
            <Name>
              <StringValue>CommandArg1</StringValue>
            </Name>
          </LookupNow>
          <LookupNow epx="Lookup">
            <Name>
              <StringValue>CommandArg2</StringValue>
            </Name>
          </LookupNow>
        </Arguments>
      </Command>
    </NodeBody>
  </Node>
</PlexilPlan>
