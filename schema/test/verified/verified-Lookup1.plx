<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="Empty">
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
    <PostCondition>
      <LookupNow epx="Lookup">
        <Name>
          <StringValue>Bar</StringValue>
        </Name>
        <Arguments>
          <RealValue>3.0</RealValue>
        </Arguments>
      </LookupNow>
    </PostCondition>
  </Node>
</PlexilPlan>
