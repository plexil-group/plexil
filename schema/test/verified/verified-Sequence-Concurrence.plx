<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" FileName="Plan.ple">
  <Node NodeType="NodeList" LineNo="2" ColNo="1">
    <NodeId>Interface</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>x</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>y</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>z</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="Sequence" LineNo="7" ColNo="22">
          <NodeId>One</NodeId>
          <InvariantCondition>
            <NoChildFailed>
              <NodeRef dir="self"/>
            </NoChildFailed>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Assignment" LineNo="8" ColNo="34">
                <NodeId>DoFirst</NodeId>
                <NodeBody>
                  <Assignment>
                    <IntegerVariable>x</IntegerVariable>
                    <NumericRHS>
                      <ADD>
                        <IntegerVariable>x</IntegerVariable>
                        <IntegerValue>1</IntegerValue>
                      </ADD>
                    </NumericRHS>
                  </Assignment>
                </NodeBody>
              </Node>
              <Node NodeType="Assignment" LineNo="9" ColNo="35">
                <NodeId>DoSecond</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">DoFirst</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Assignment>
                    <IntegerVariable>y</IntegerVariable>
                    <NumericRHS>
                      <ADD>
                        <IntegerVariable>y</IntegerVariable>
                        <IntegerValue>1</IntegerValue>
                      </ADD>
                    </NumericRHS>
                  </Assignment>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
        <Node NodeType="NodeList" epx="Concurrence" LineNo="11" ColNo="24">
          <NodeId>Two</NodeId>
          <NodeBody>
            <NodeList>
              <Node NodeType="Assignment" LineNo="12" ColNo="38">
                <NodeId>DoFirstDiff</NodeId>
                <NodeBody>
                  <Assignment>
                    <IntegerVariable>z</IntegerVariable>
                    <NumericRHS>
                      <ADD>
                        <IntegerVariable>z</IntegerVariable>
                        <IntegerValue>1</IntegerValue>
                      </ADD>
                    </NumericRHS>
                  </Assignment>
                </NodeBody>
              </Node>
              <Node NodeType="Assignment" LineNo="13" ColNo="35">
                <NodeId>DoSecond</NodeId>
                <NodeBody>
                  <Assignment>
                    <IntegerVariable>y</IntegerVariable>
                    <NumericRHS>
                      <ADD>
                        <IntegerVariable>y</IntegerVariable>
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
