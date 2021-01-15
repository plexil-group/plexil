<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="Sequence">
    <NodeId generated="1">ep2cp_OnMessage_d13e11</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>ep2cp_hdl</Name>
        <Type>String</Type>
      </DeclareVariable>
    </VariableDeclarations>
    <UsingMutex>
      <Name>m</Name>
    </UsingMutex>
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
                <StringValue>ReceiveMessage</StringValue>
              </Name>
              <Arguments>
                <StringValue>moveRover</StringValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="NodeList" epx="aux">
          <NodeId generated="1">ep2cp_MsgAction_</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CmdWait</NodeRef>
            </Finished>
          </StartCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Empty">
                <NodeId>foo</NodeId>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
