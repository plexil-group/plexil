<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <Node NodeType="NodeList" epx="OnCommand">
    <NodeId generated="1">ep2cp_OnCommand_d13e3</NodeId>
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
    <NodeBody>
      <NodeList>
        <Node NodeType="Command" epx="OnCommand-command-wait">
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
                <StringValue>moveRover</StringValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Command" epx="OnCommand-get-param">
          <NodeId generated="1">ep2cp_CmdGetParam_0</NodeId>
          <StartCondition>
            <IsKnown>
              <StringVariable>ep2cp_hdl</StringVariable>
            </IsKnown>
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
        <Node NodeType="Command" epx="OnCommand-get-param">
          <NodeId generated="1">ep2cp_CmdGetParam_1</NodeId>
          <StartCondition>
            <IsKnown>
              <StringVariable>ep2cp_hdl</StringVariable>
            </IsKnown>
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
        <Node NodeType="Empty" epx="OnCommand-action">
          <NodeId>foo</NodeId>
          <StartCondition>
            <AND>
              <Succeeded>
                <NodeRef dir="sibling">ep2cp_CmdGetParam_0</NodeRef>
              </Succeeded>
              <Succeeded>
                <NodeRef dir="sibling">ep2cp_CmdGetParam_1</NodeRef>
              </Succeeded>
            </AND>
          </StartCondition>
        </Node>
        <Node NodeType="Command" epx="OnCommand-return">
          <NodeId generated="1">ep2cp_OnCommandReturn_moveRover</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">foo</NodeRef>
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
