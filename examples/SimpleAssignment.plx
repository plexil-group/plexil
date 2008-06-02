  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="plexil.xsd">
      <Node NodeType="Assignment">
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>foo</Name>
                  <Type>Integer</Type>
                  <InitialValue>
                      <IntegerValue>0</IntegerValue>
                  </InitialValue>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeId>SimpleAssignment</NodeId>
          <PostCondition>
              <EQNumeric>
                  <IntegerVariable>foo</IntegerVariable>
                  <IntegerValue>3</IntegerValue>
              </EQNumeric>
          </PostCondition>
          <NodeBody>
              <Assignment>
                  <IntegerVariable>foo</IntegerVariable>
                  <NumericRHS>
                      <IntegerValue>3</IntegerValue>
                  </NumericRHS>
              </Assignment>
          </NodeBody>
      </Node>
  </PlexilPlan>
