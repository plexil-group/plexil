<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan>
  <GlobalDeclarations>
    <MutexDeclaration>
      <Name>m</Name>
    </MutexDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>ep2cp_If_d1e12</NodeId>
    <Mutexes>
      <Name>
        <StringValue>m</StringValue>
      </Name>
    </Mutexes>
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
