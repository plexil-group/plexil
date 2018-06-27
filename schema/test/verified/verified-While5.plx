<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <GlobalDeclarations>
    <StateDeclaration>
      <Name>flag</Name>
      <Return>
        <Name>_return_0</Name>
        <Type>Boolean</Type>
      </Return>
    </StateDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="While">
    <NodeId>Root</NodeId>
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
            <LookupNow>
              <Name>
                <StringValue>flag</StringValue>
              </Name>
            </LookupNow>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Action">
          <NodeId>A</NodeId>
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
</PlexilPlan>
