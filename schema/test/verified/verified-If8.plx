<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator">
  <GlobalDeclarations>
    <StateDeclaration>
      <Name>X</Name>
      <Return>
        <Name>_return_0</Name>
        <Type>Integer</Type>
      </Return>
    </StateDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="If">
    <NodeId>Root</NodeId>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId>ep2cp_IfTest</NodeId>
          <PostCondition>
            <EQNumeric>
              <IntegerValue>2</IntegerValue>
              <LookupNow>
                <Name>
                  <StringValue>X</StringValue>
                </Name>
              </LookupNow>
            </EQNumeric>
          </PostCondition>
        </Node>
        <Node NodeType="Empty" epx="Then">
          <NodeId>A</NodeId>
          <StartCondition>
            <Succeeded>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </Succeeded>
          </StartCondition>
          <SkipCondition>
            <PostconditionFailed>
              <NodeRef dir="sibling">ep2cp_IfTest</NodeRef>
            </PostconditionFailed>
          </SkipCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
