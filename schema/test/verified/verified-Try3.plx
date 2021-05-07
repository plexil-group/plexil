<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="Try">
    <NodeId generated="1">ep2cp_Try_d13e3</NodeId>
    <EndCondition>
      <OR>
        <Succeeded>
          <NodeRef dir="child">One</NodeRef>
        </Succeeded>
        <Finished>
          <NodeRef dir="child">Two</NodeRef>
        </Finished>
      </OR>
    </EndCondition>
    <PostCondition>
      <OR>
        <Succeeded>
          <NodeRef dir="child">One</NodeRef>
        </Succeeded>
        <Succeeded>
          <NodeRef dir="child">Two</NodeRef>
        </Succeeded>
      </OR>
    </PostCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty">
          <NodeId>One</NodeId>
        </Node>
        <Node NodeType="Empty">
          <NodeId>Two</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">One</NodeRef>
            </Finished>
          </StartCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
