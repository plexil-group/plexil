<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
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
    <UsingMutex>
      <Name>m</Name>
    </UsingMutex>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="aux">
          <NodeId generated="1">ep2cp_WhileBody</NodeId>
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
                  <BooleanVariable>foo</BooleanVariable>
                </PostCondition>
              </Node>
              <Node NodeType="Empty" epx="Action">
                <NodeId>One</NodeId>
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
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
