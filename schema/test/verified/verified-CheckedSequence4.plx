<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="CheckedSequence">
    <NodeId>ep2cp_CheckedSequence_d1e3</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <InvariantCondition>
      <NOT>
        <OR>
          <Failed>
            <NodeRef dir="child">One</NodeRef>
          </Failed>
          <Failed>
            <NodeRef dir="child">ep2cp_CheckedSequence_d1e29</NodeRef>
          </Failed>
          <Failed>
            <NodeRef dir="child">Three</NodeRef>
          </Failed>
        </OR>
      </NOT>
    </InvariantCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty">
          <NodeId>One</NodeId>
        </Node>
        <Node NodeType="NodeList" epx="CheckedSequence">
          <NodeId>ep2cp_CheckedSequence_d1e29</NodeId>
          <StartCondition>
            <AND>
              <Finished>
                <NodeRef dir="sibling">One</NodeRef>
              </Finished>
              <EQNumeric>
                <IntegerVariable>foo</IntegerVariable>
                <IntegerValue>0</IntegerValue>
              </EQNumeric>
            </AND>
          </StartCondition>
          <InvariantCondition>
            <NOT>
              <Failed>
                <NodeRef dir="child">Two</NodeRef>
              </Failed>
            </NOT>
          </InvariantCondition>
          <NodeBody>
            <NodeList>
              <Node NodeType="Empty">
                <NodeId>Two</NodeId>
              </Node>
            </NodeList>
          </NodeBody>
        </Node>
        <Node NodeType="Empty">
          <NodeId>Three</NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">ep2cp_CheckedSequence_d1e29</NodeRef>
            </Finished>
          </StartCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
