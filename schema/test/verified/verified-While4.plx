<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="While">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>i</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="While_wrapper">
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
                  <LT>
                    <IntegerVariable>i</IntegerVariable>
                    <IntegerValue>5</IntegerValue>
                  </LT>
                </PostCondition>
              </Node>
              <Node NodeType="Assignment" epx="Action">
                <NodeId>increment</NodeId>
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
                <NodeBody>
                  <Assignment>
                    <IntegerVariable>i</IntegerVariable>
                    <NumericRHS>
                      <ADD>
                        <IntegerVariable>i</IntegerVariable>
                        <IntegerValue>1</IntegerValue>
                      </ADD>
                    </NumericRHS>
                  </Assignment>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
