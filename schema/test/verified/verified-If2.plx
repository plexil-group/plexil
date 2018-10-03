<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="If">
    <NodeId>ep2cp_If_d1e3</NodeId>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId>ep2cp_IfTest</NodeId>
          <PostCondition>
            <BooleanValue>false</BooleanValue>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>One</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </PostconditionFailed>
          </SkipCondition>
        </Node>
        <Node NodeType="Empty" epx="Else">
          <NodeId>Two</NodeId>
          <StartCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </PostconditionFailed>
          </StartCondition>
          <SkipCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </Succeeded>
          </SkipCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
