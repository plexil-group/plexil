<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="CheckedSequence">
    <NodeId>ep2cp_CheckedSequence_d1e3</NodeId>
    <InvariantCondition>
      <NOT>
        <OR>
          <Failed>
            <NodeRef dir="child">One</NodeRef>
          </Failed>
          <Failed>
            <NodeRef dir="child">Two</NodeRef>
          </Failed>
        </OR>
      </NOT>
    </InvariantCondition>
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
