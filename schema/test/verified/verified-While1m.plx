<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan>
  <GlobalDeclarations>
    <MutexDeclaration>
      <Name>m</Name>
    </MutexDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="While">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>true</BooleanValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <Mutexes>
      <Name>
        <StringValue>m</StringValue>
      </Name>
    </Mutexes>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="aux">
          <NodeId>ep2cp_WhileBody</NodeId>
          <RepeatCondition>
            <EQInternal>
              <NodeOutcomeVariable>
                <NodeRef dir="child">ep2cp_WhileTest</NodeRef>
              </NodeOutcomeVariable>
              <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
            </EQInternal>
          </RepeatCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Empty" epx="Condition">
                <NodeId>ep2cp_WhileTest</NodeId>
                <PostCondition>
                  <BooleanVariable>foo</BooleanVariable>
                </PostCondition>
              </Node>
              <Node NodeType="Empty" epx="Action">
                <NodeId>One</NodeId>
                <Mutexes>
                  <Name>
                    <StringValue>m</StringValue>
                  </Name>
                </Mutexes>
                <StartCondition>
                  <EQInternal>
                    <NodeOutcomeVariable>
                      <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
                    </NodeOutcomeVariable>
                    <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
                  </EQInternal>
                </StartCondition>
                <SkipCondition>
                  <AND>
                    <EQInternal>
                      <NodeStateVariable>
                        <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
                      </NodeStateVariable>
                      <NodeStateValue>FINISHED</NodeStateValue>
                    </EQInternal>
                    <EQInternal>
                      <NodeFailureVariable>
                        <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
                      </NodeFailureVariable>
                      <NodeFailureValue>POST_CONDITION_FAILED</NodeFailureValue>
                    </EQInternal>
                  </AND>
                </SkipCondition>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
