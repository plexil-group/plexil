<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="If">
    <NodeId generated="1">ep2cp_If_d13e3</NodeId>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Then">
          <StartCondition>
            <BooleanValue>false</BooleanValue>
          </StartCondition>
          <SkipCondition>
            <NOT>
              <BooleanValue>false</BooleanValue>
            </NOT>
          </SkipCondition>
          <NodeId>One</NodeId>
        </Node>
        <Node NodeType="Empty" epx="Else">
          <StartCondition>
            <NOT>
              <BooleanValue>false</BooleanValue>
            </NOT>
          </StartCondition>
          <SkipCondition>
            <BooleanValue>false</BooleanValue>
          </SkipCondition>
          <NodeId>Two</NodeId>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
