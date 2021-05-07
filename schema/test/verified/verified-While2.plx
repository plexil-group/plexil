<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="While">
    <NodeId generated="1">ep2cp_While_d13e3</NodeId>
    <RepeatCondition>
      <Succeeded>
        <NodeRef dir="child">ep2cp_WhileTest</NodeRef>
      </Succeeded>
    </RepeatCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId generated="1">ep2cp_WhileTest</NodeId>
          <PostCondition>
            <LT>
              <IntegerValue>0</IntegerValue>
              <IntegerValue>1</IntegerValue>
            </LT>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Action">
          <NodeId>One</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
            </PostconditionFailed>
          </SkipCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
