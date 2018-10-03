<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="Sequence">
    <NodeId>ep2cp_Sequence_d1e3</NodeId>
    <InvariantCondition>
      <NOT>
        <OR>
          <AND>
            <Finished>
              <NodeRef dir="child">One</NodeRef>
            </Finished>
            <EQInternal>
              <NodeOutcomeVariable>
                <NodeRef dir="child">One</NodeRef>
              </NodeOutcomeVariable>
              <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
            </EQInternal>
          </AND>
          <AND>
            <Finished>
              <NodeRef dir="child">Two</NodeRef>
            </Finished>
            <EQInternal>
              <NodeOutcomeVariable>
                <NodeRef dir="child">Two</NodeRef>
              </NodeOutcomeVariable>
              <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
            </EQInternal>
          </AND>
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
