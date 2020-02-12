<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <GlobalDeclarations>
    <DeclareMutex>
      <Name>m</Name>
    </DeclareMutex>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="Sequence">
    <NodeId>ep2cp_Sequence_d2e1</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>distance</Name>
        <Type>Integer</Type>
      </DeclareVariable>
      <DeclareVariable>
        <Name>direction</Name>
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
    <UsingMutex>
      <Name>m</Name>
    </UsingMutex>
    <NodeBody>
      <NodeList>
        <Node NodeType="Command">
          <NodeId>ep2cp_CmdWait</NodeId>
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
                <StringValue>moveRover</StringValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Command">
          <NodeId>ep2cp_CmdGetParam_distance</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CmdWait</NodeRef>
            </Finished>
          </StartCondition>
          <EndCondition>
            <IsKnown>
              <IntegerVariable>distance</IntegerVariable>
            </IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <IntegerVariable>distance</IntegerVariable>
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
        <Node NodeType="Command">
          <NodeId>ep2cp_CmdGetParam_direction</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CmdGetParam_distance</NodeRef>
            </Finished>
          </StartCondition>
          <EndCondition>
            <IsKnown>
              <RealVariable>direction</RealVariable>
            </IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <RealVariable>direction</RealVariable>
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
        <Node NodeType="NodeList">
          <NodeId>ep2cp_CmdAction_moveRover</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CmdGetParam_direction</NodeRef>
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
        <Node NodeType="Command">
          <NodeId>ep2cp_CmdReturn</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CmdAction_moveRover</NodeRef>
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
</PlexilPlan>
