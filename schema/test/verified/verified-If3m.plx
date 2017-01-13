<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan>
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>ep2cp_If_d1e12</NodeId>
    <UsingMutex>
      <Name>
        <StringValue>m</StringValue>
      </Name>
    </UsingMutex>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId>ep2cp_IfTest</NodeId>
          <PostCondition>
            <BooleanValue>true</BooleanValue>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>One</NodeId>
          <StartCondition>
            <EQInternal>
              <NodeOutcomeVariable>
                <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
              </NodeOutcomeVariable>
              <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
            </EQInternal>
          </StartCondition>
          <SkipCondition>
            <EQInternal>
              <NodeFailureVariable>
                <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
              </NodeFailureVariable>
              <NodeFailureValue>POST_CONDITION_FAILED</NodeFailureValue>
            </EQInternal>
          </SkipCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
