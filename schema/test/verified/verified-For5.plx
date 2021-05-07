<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="For">
    <NodeId>Root</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>total</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
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
        <Node NodeType="NodeList" epx="For-body">
          <NodeId generated="1">ep2cp_ForLoop</NodeId>
          <SkipCondition>
            <NOT>
              <LT>
                <IntegerVariable>i</IntegerVariable>
                <IntegerValue>3</IntegerValue>
              </LT>
            </NOT>
          </SkipCondition>
          <RepeatCondition>
            <BooleanValue>true</BooleanValue>
          </RepeatCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="NodeList" epx="While">
                <NodeId generated="1">ep2cp_While_d13e60</NodeId>
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
                          <IntegerVariable>total</IntegerVariable>
                          <MUL>
                            <IntegerVariable>i</IntegerVariable>
                            <IntegerValue>5</IntegerValue>
                          </MUL>
                        </LT>
                      </PostCondition>
                    </Node>
                    <Node NodeType="Assignment" epx="Action">
                      <NodeId>Inner</NodeId>
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
                          <IntegerVariable>total</IntegerVariable>
                          <NumericRHS>
                            <ADD>
                              <IntegerVariable>total</IntegerVariable>
                              <IntegerValue>1</IntegerValue>
                            </ADD>
                          </NumericRHS>
                        </Assignment>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
              <Node NodeType="Assignment" epx="LoopVariableUpdate">
                <NodeId generated="1">ep2cp_ForLoopUpdater</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_While_d13e60</NodeRef>
                  </Finished>
                </StartCondition>
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
