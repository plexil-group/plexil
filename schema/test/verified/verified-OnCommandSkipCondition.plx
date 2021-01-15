<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="Concurrence">
    <NodeId>TestSequence</NodeId>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="Sequence">
          <NodeId>Recv1</NodeId>
          <VariableDeclarations>
            <DeclareVariable>
              <Name>arg1</Name>
              <Type>String</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg2</Name>
              <Type>Boolean</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg3</Name>
              <Type>Integer</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg4</Name>
              <Type>Real</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>ep2cp_hdl</Name>
              <Type>String</Type>
            </DeclareVariable>
          </VariableDeclarations>
          <InvariantCondition>
            <NoChildFailed>
              <NodeRef dir="self"/>
            </NoChildFailed>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Command" epx="aux">
                <NodeId generated="1">ep2cp_CmdWait</NodeId>
                <EndCondition>
                  <IsKnown>
                    <StringVariable>ep2cp_hdl</StringVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <StringVariable>ep2cp_hdl</StringVariable>
                    <Name>
                      <StringValue>ReceiveCommand</StringValue>
                    </Name>
                    <Arguments>
                      <StringValue>recv_test</StringValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg1</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdWait</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <StringVariable>arg1</StringVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <StringVariable>arg1</StringVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>0</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg2</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg1</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <BooleanVariable>arg2</BooleanVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <BooleanVariable>arg2</BooleanVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>1</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg3</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg2</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <IntegerVariable>arg3</IntegerVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <IntegerVariable>arg3</IntegerVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>2</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg4</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg3</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <RealVariable>arg4</RealVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <RealVariable>arg4</RealVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>3</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="NodeList" epx="aux">
                <NodeId generated="1">ep2cp_CmdAction_recv_test</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg4</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <NodeList>
                    <Node NodeType="Command">
                      <NodeId>Recv1__CHILD__1</NodeId>
                      <NodeBody>
                        <Command>
                          <Name>
                            <StringValue>pprint</StringValue>
                          </Name>
                          <Arguments>
                            <StringValue>Received</StringValue>
                            <StringVariable>arg1</StringVariable>
                            <BooleanVariable>arg2</BooleanVariable>
                            <IntegerVariable>arg3</IntegerVariable>
                            <RealVariable>arg4</RealVariable>
                          </Arguments>
                        </Command>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="aux">
                <NodeId generated="1">ep2cp_CmdReturn</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdAction_recv_test</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Command>
                    <Name>
                      <StringValue>SendReturnValue</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <BooleanValue>true</BooleanValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
        <Node NodeType="NodeList" epx="Sequence">
          <NodeId>Recv2</NodeId>
          <VariableDeclarations>
            <DeclareVariable>
              <Name>arg1</Name>
              <Type>String</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg2</Name>
              <Type>Boolean</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg3</Name>
              <Type>Integer</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>arg4</Name>
              <Type>Real</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>ep2cp_hdl</Name>
              <Type>String</Type>
            </DeclareVariable>
          </VariableDeclarations>
          <SkipCondition>
            <BooleanValue>true</BooleanValue>
          </SkipCondition>
          <InvariantCondition>
            <NoChildFailed>
              <NodeRef dir="self"/>
            </NoChildFailed>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Command" epx="aux">
                <NodeId generated="1">ep2cp_CmdWait</NodeId>
                <EndCondition>
                  <IsKnown>
                    <StringVariable>ep2cp_hdl</StringVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <StringVariable>ep2cp_hdl</StringVariable>
                    <Name>
                      <StringValue>ReceiveCommand</StringValue>
                    </Name>
                    <Arguments>
                      <StringValue>recv_test</StringValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg1</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdWait</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <StringVariable>arg1</StringVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <StringVariable>arg1</StringVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>0</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg2</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg1</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <BooleanVariable>arg2</BooleanVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <BooleanVariable>arg2</BooleanVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>1</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg3</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg2</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <IntegerVariable>arg3</IntegerVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <IntegerVariable>arg3</IntegerVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>2</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="OnCommand_param">
                <NodeId generated="1">ep2cp_CmdGetParam_arg4</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg3</NodeRef>
                  </Finished>
                </StartCondition>
                <EndCondition>
                  <IsKnown>
                    <RealVariable>arg4</RealVariable>
                  </IsKnown>
                </EndCondition>
                <NodeBody>
                  <Command>
                    <RealVariable>arg4</RealVariable>
                    <Name>
                      <StringValue>GetParameter</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <IntegerValue>3</IntegerValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
              <Node NodeType="NodeList" epx="aux">
                <NodeId generated="1">ep2cp_CmdAction_recv_test</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdGetParam_arg4</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <NodeList>
                    <Node NodeType="Command">
                      <NodeId>Recv2__CHILD__1</NodeId>
                      <NodeBody>
                        <Command>
                          <Name>
                            <StringValue>pprint</StringValue>
                          </Name>
                          <Arguments>
                            <StringValue>Received</StringValue>
                            <StringVariable>arg1</StringVariable>
                            <BooleanVariable>arg2</BooleanVariable>
                            <IntegerVariable>arg3</IntegerVariable>
                            <RealVariable>arg4</RealVariable>
                          </Arguments>
                        </Command>
                      </NodeBody>
                    </Node>
                  </NodeList>
                </NodeBody>
              </Node>
              <Node NodeType="Command" epx="aux">
                <NodeId generated="1">ep2cp_CmdReturn</NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">ep2cp_CmdAction_recv_test</NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Command>
                    <Name>
                      <StringValue>SendReturnValue</StringValue>
                    </Name>
                    <Arguments>
                      <StringVariable>ep2cp_hdl</StringVariable>
                      <BooleanValue>true</BooleanValue>
                    </Arguments>
                  </Command>
                </NodeBody>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
