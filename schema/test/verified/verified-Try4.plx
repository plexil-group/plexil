<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <Node NodeType="NodeList" epx="Try">
    <NodeId>ep2cp_Try_d1e3</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <EndCondition>
      <OR>
        <AND>
          <Finished>
            <NodeRef dir="child">One</NodeRef>
          </Finished>
          <EQInternal>
            <NodeOutcomeVariable>
              <NodeRef dir="child">One</NodeRef>
            </NodeOutcomeVariable>
            <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
          </EQInternal>
        </AND>
        <AND>
          <Finished>
            <NodeRef dir="child">ep2cp_Try_d1e29</NodeRef>
          </Finished>
          <EQInternal>
            <NodeOutcomeVariable>
              <NodeRef dir="child">ep2cp_Try_d1e29</NodeRef>
            </NodeOutcomeVariable>
            <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
          </EQInternal>
        </AND>
        <Finished>
          <NodeRef dir="child">Three</NodeRef>
        </Finished>
      </OR>
    </EndCondition>
    <PostCondition>
      <OR>
        <EQInternal>
          <NodeOutcomeVariable>
            <NodeRef dir="child">One</NodeRef>
          </NodeOutcomeVariable>
          <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
        </EQInternal>
        <EQInternal>
          <NodeOutcomeVariable>
            <NodeRef dir="child">ep2cp_Try_d1e29</NodeRef>
          </NodeOutcomeVariable>
          <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
        </EQInternal>
        <EQInternal>
          <NodeOutcomeVariable>
            <NodeRef dir="child">Three</NodeRef>
          </NodeOutcomeVariable>
          <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
        </EQInternal>
      </OR>
    </PostCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty">
          <NodeId>One</NodeId>
        </Node>
        <Node NodeType="NodeList" epx="Try">
          <NodeId>ep2cp_Try_d1e29</NodeId>
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
          <PostCondition>
            <EQInternal>
              <NodeOutcomeVariable>
                <NodeRef dir="child">Two</NodeRef>
              </NodeOutcomeVariable>
              <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
            </EQInternal>
          </PostCondition>
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
              <NodeRef dir="sibling">ep2cp_Try_d1e29</NodeRef>
            </Finished>
          </StartCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
