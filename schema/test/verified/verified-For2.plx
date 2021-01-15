<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="For">
    <NodeId generated="1">ep2cp_For_d13e3</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>count</Name>
        <Type>Real</Type>
        <InitialValue>
          <RealValue>0.0</RealValue>
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
                <RealVariable>count</RealVariable>
                <RealValue>1.0</RealValue>
              </LT>
            </NOT>
          </SkipCondition>
          <RepeatCondition>
            <BooleanValue>true</BooleanValue>
          </RepeatCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Empty">
                <NodeId>One</NodeId>
              </Node>
              <Node NodeType="Assignment" epx="LoopVariableUpdate">
                <NodeId generated="1">ep2cp_ForLoopUpdater</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">One</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Assignment>
                    <RealVariable>count</RealVariable>
                    <NumericRHS>
                      <ADD>
                        <RealVariable>count</RealVariable>
                        <RealValue>0.2</RealValue>
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
