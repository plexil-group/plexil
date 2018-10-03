<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="Try">
    <NodeId>ep2cp_Try_d1e3</NodeId>
    <EndCondition>
      <OR>
        <AND>
          <Finished>
            <NodeRef dir="child">One</NodeRef>
          </Finished>
          <EQInternal>
            <NodeOutcomeVariable>
              <NodeRef dir="child">One</NodeRef>
            </NodeOutcomeVariable>
            <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
          </EQInternal>
        </AND>
        <Finished>
          <NodeRef dir="child">Two</NodeRef>
        </Finished>
      </OR>
    </EndCondition>
    <PostCondition>
      <OR>
        <EQInternal>
          <NodeOutcomeVariable>
            <NodeRef dir="child">One</NodeRef>
          </NodeOutcomeVariable>
          <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
        </EQInternal>
        <EQInternal>
          <NodeOutcomeVariable>
            <NodeRef dir="child">Two</NodeRef>
          </NodeOutcomeVariable>
          <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
        </EQInternal>
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
