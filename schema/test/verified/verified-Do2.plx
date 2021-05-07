<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <GlobalDeclarations>
    <CommandDeclaration>
      <Name>bar</Name>
      <Return>
        <Type>Boolean</Type>
      </Return>
    </CommandDeclaration>
  </GlobalDeclarations>
  <Node NodeType="NodeList" epx="Concurrence">
    <NodeId>Do2</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>foo</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>true</BooleanValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>
        <Node NodeType="Command">
          <NodeId>invoke-bar</NodeId>
          <NodeBody>
            <Command>
              <BooleanVariable>foo</BooleanVariable>
              <Name>
                <StringValue>bar</StringValue>
              </Name>
            </Command>
          </NodeBody>
        </Node>
        <Node epx="Do" NodeType="Empty">
          <NodeId>Do1</NodeId>
          <RepeatCondition>
            <BooleanVariable>foo</BooleanVariable>
          </RepeatCondition>
        </Node>
      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
