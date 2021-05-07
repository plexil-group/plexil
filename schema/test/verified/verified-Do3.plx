<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <CommandDeclaration>
      <Name>pprint</Name>
      <AnyParameters/>
    </CommandDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="Sequence">
    <NodeId>Do3</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>i</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <InvariantCondition>
      <NoChildFailed>
        <NodeRef dir="self"/>
      </NoChildFailed>
    </InvariantCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Command">
          <NodeId>print-a-message</NodeId>
          <NodeBody>
            <Command>
              <Name>
                <StringValue>pprint</StringValue>
              </Name>
              <Arguments>
                <StringValue>Begin do loop test</StringValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
        <Node epx="Do" NodeType="NodeList">
          <NodeId>outer-do</NodeId>
          <VariableDeclarations>
            <DeclareVariable>
              <Name>j</Name>
              <Type>Integer</Type>
              <InitialValue>
                <IntegerValue>0</IntegerValue>
              </InitialValue>
            </DeclareVariable>
          </VariableDeclarations>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">print-a-message</NodeRef>
            </Finished>
          </StartCondition>
          <RepeatCondition>
            <LT>
              <IntegerVariable>i</IntegerVariable>
              <IntegerValue>4</IntegerValue>
            </LT>
          </RepeatCondition>
          <InvariantCondition>
            <NoChildFailed>
              <NodeRef dir="self"/>
            </NoChildFailed>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node epx="Do" NodeType="NodeList">
                <NodeId>inner-do</NodeId>
                <RepeatCondition>
                  <LT>
                    <IntegerVariable>j</IntegerVariable>
                    <IntegerValue>5</IntegerValue>
                  </LT>
                </RepeatCondition>
                <InvariantCondition>
                  <NoChildFailed>
                    <NodeRef dir="self"/>
                  </NoChildFailed>
                </InvariantCondition>
                <NodeBody>
                  <NodeList>
                    <Node NodeType="Assignment">
                      <NodeId generated="1">ep2cp_Node_d13e82</NodeId>
                      <NodeBody>
                        <Assignment>
                          <IntegerVariable>j</IntegerVariable>
                          <NumericRHS>
                            <ADD>
                              <IntegerVariable>j</IntegerVariable>
                              <IntegerValue>1</IntegerValue>
                            </ADD>
                          </NumericRHS>
                        </Assignment>
                      </NodeBody>
                    </Node>
                    <Node NodeType="Command">
                      <NodeId>print-inner-loop-message</NodeId>
                      <StartCondition>
                        <Finished>
                          <NodeRef dir="sibling">ep2cp_Node_d13e82</NodeRef>
                        </Finished>
                      </StartCondition>
                      <NodeBody>
                        <Command>
                          <Name>
                            <StringValue>pprint</StringValue>
                          </Name>
                          <Arguments>
                            <StringValue>  j is</StringValue>
                            <IntegerVariable>j</IntegerVariable>
                          </Arguments>
                        </Command>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
              <Node NodeType="Assignment">
                <NodeId generated="1">ep2cp_Node_d13e137</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">inner-do</NodeRef>
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
              <Node NodeType="Command">
                <NodeId>print-outer-loop-message</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_Node_d13e137</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Command>
                    <Name>
                      <StringValue>pprint</StringValue>
                    </Name>
                    <Arguments>
                      <StringValue>i is</StringValue>
                      <IntegerVariable>i</IntegerVariable>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
        <Node NodeType="Command">
          <NodeId>print-exit-message</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">outer-do</NodeRef>
            </Finished>
          </StartCondition>
          <NodeBody>
            <Command>
              <Name>
                <StringValue>pprint</StringValue>
              </Name>
              <Arguments>
                <StringValue>Done.</StringValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
