<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
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
      <Succeeded>
        <NodeRef dir="child">ep2cp_WhileTest</NodeRef>
      </Succeeded>
    </RepeatCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId generated="1">ep2cp_WhileTest</NodeId>
          <PostCondition>
            <LookupNow epx="Lookup">
              <Name>
                <StringValue>flag</StringValue>
              </Name>
            </LookupNow>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Action">
          <NodeId>A</NodeId>
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
