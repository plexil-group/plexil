<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="If">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId generated="1">ep2cp_IfTest</NodeId>
          <PostCondition>
            <EQNumeric>
              <IntegerVariable>foo</IntegerVariable>
              <IntegerValue>1</IntegerValue>
            </EQNumeric>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>One</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </PostconditionFailed>
          </SkipCondition>
        </Node>
        <Node NodeType="Empty" epx="ElseIf">
          <NodeId generated="1">ep2cp_ElseIf-1</NodeId>
          <StartCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </PostconditionFailed>
          </StartCondition>
          <SkipCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </Succeeded>
          </SkipCondition>
          <PostCondition>
            <EQNumeric>
              <IntegerVariable>foo</IntegerVariable>
              <IntegerValue>2</IntegerValue>
            </EQNumeric>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>Two</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <OR>
              <Skipped>
                <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
              </Skipped>
              <PostconditionFailed>
                <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
              </PostconditionFailed>
            </OR>
          </SkipCondition>
        </Node>
        <Node NodeType="Empty" epx="ElseIf">
          <NodeId generated="1">ep2cp_ElseIf-2</NodeId>
          <StartCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
            </PostconditionFailed>
          </StartCondition>
          <SkipCondition>
            <OR>
              <Skipped>
                <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
              </Skipped>
              <Succeeded>
                <NodeRef dir="sibling">ep2cp_ElseIf-1</NodeRef>
              </Succeeded>
            </OR>
          </SkipCondition>
          <PostCondition>
            <EQNumeric>
              <IntegerVariable>foo</IntegerVariable>
              <IntegerValue>3</IntegerValue>
            </EQNumeric>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>Three</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <OR>
              <Skipped>
                <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
              </Skipped>
              <PostconditionFailed>
                <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
              </PostconditionFailed>
            </OR>
          </SkipCondition>
        </Node>
        <Node NodeType="Empty" epx="Else">
          <NodeId>Four</NodeId>
          <StartCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
            </PostconditionFailed>
          </StartCondition>
          <SkipCondition>
            <OR>
              <Skipped>
                <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
              </Skipped>
              <Succeeded>
                <NodeRef dir="sibling">ep2cp_ElseIf-2</NodeRef>
              </Succeeded>
            </OR>
          </SkipCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
