<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="Sequence">
    <NodeId generated="1">ep2cp_Sequence_d13e3</NodeId>
    <VariableDeclarations>
      <!-- This is the array used for command FOO: -->
      <DeclareArray>
        <Name>x</Name>
        <Type>Integer</Type>
        <MaxSize>4</MaxSize>
      </DeclareArray>
    </VariableDeclarations>
    <InvariantCondition>
      <NoChildFailed>
        <NodeRef dir="self"/>
      </NoChildFailed>
    </InvariantCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="SynchronousCommand" FileName="foo.ple" LineNo="104" ColNo="1">
          <NodeId>A</NodeId>
          <NodeBody>
            <NodeList>
              <Node NodeType="NodeList" epx="aux">
                <NodeId generated="1">ep2cp_SynchronousCommandAux</NodeId>
                <VariableDeclarations>
                  <DeclareArray>
                    <Name>ep2cp_return</Name>
                    <Type>Integer</Type>
                    <MaxSize>4</MaxSize>
                  </DeclareArray>
                </VariableDeclarations>
                <NodeBody>
                  <NodeList>
                    <Node NodeType="Command" epx="aux">
                      <NodeId generated="1">ep2cp_SynchronousCommandCommand</NodeId>
                      <EndCondition>
                        <IsKnown>
                          <ArrayElement>
                            <ArrayVariable>ep2cp_return</ArrayVariable>
                            <Index>
                              <IntegerValue>0</IntegerValue>
                            </Index>
                          </ArrayElement>
                        </IsKnown>
                      </EndCondition>
                      <NodeBody>
                        <Command>
                          <ResourceList>
                            <Resource>
                              <ResourceName>
                                <StringValue>arm</StringValue>
                              </ResourceName>
                              <ResourcePriority>
                                <IntegerValue>3</IntegerValue>
                              </ResourcePriority>
                            </Resource>
                          </ResourceList>
                          <ArrayVariable>ep2cp_return</ArrayVariable>
                          <Name>
                            <StringValue>FOO</StringValue>
                          </Name>
                        </Command>
                      </NodeBody>
                    </Node>
                    <Node NodeType="Assignment" epx="aux">
                      <NodeId generated="1">ep2cp_SynchronousCommandAssignment</NodeId>
                      <StartCondition>
                        <Finished>
                          <NodeRef dir="sibling">ep2cp_SynchronousCommandCommand</NodeRef>
                        </Finished>
                      </StartCondition>
                      <NodeBody>
                        <Assignment>
                          <ArrayVariable>x</ArrayVariable>
                          <ArrayRHS>
                            <ArrayVariable>ep2cp_return</ArrayVariable>
                          </ArrayRHS>
                        </Assignment>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
        <Node NodeType="NodeList" epx="SynchronousCommand">
          <NodeId>B</NodeId>
          <VariableDeclarations>
            <!-- This is the array used for command BAR: -->
            <DeclareArray>
              <Name>x</Name>
              <Type>String</Type>
              <MaxSize>20</MaxSize>
            </DeclareArray>
          </VariableDeclarations>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">A</NodeRef>
            </Finished>
          </StartCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="NodeList" epx="aux">
                <NodeId generated="1">ep2cp_SynchronousCommandAux</NodeId>
                <VariableDeclarations>
                  <DeclareArray>
                    <Name>ep2cp_return</Name>
                    <Type>String</Type>
                    <MaxSize>20</MaxSize>
                  </DeclareArray>
                </VariableDeclarations>
                <NodeBody>
                  <NodeList>
                    <Node NodeType="Command" epx="aux">
                      <NodeId generated="1">ep2cp_SynchronousCommandCommand</NodeId>
                      <EndCondition>
                        <IsKnown>
                          <ArrayElement>
                            <ArrayVariable>ep2cp_return</ArrayVariable>
                            <Index>
                              <IntegerValue>0</IntegerValue>
                            </Index>
                          </ArrayElement>
                        </IsKnown>
                      </EndCondition>
                      <NodeBody>
                        <Command>
                          <ResourceList/>
                          <ArrayVariable>ep2cp_return</ArrayVariable>
                          <Name>
                            <StringValue>BAR</StringValue>
                          </Name>
                        </Command>
                      </NodeBody>
                    </Node>
                    <Node NodeType="Assignment" epx="aux">
                      <NodeId generated="1">ep2cp_SynchronousCommandAssignment</NodeId>
                      <StartCondition>
                        <Finished>
                          <NodeRef dir="sibling">ep2cp_SynchronousCommandCommand</NodeRef>
                        </Finished>
                      </StartCondition>
                      <NodeBody>
                        <Assignment>
                          <ArrayVariable>x</ArrayVariable>
                          <ArrayRHS>
                            <ArrayVariable>ep2cp_return</ArrayVariable>
                          </ArrayRHS>
                        </Assignment>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
