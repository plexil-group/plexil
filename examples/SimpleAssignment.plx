  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="SimpleAssignment.ple">
      <Node NodeType="Assignment" FileName="SimpleAssignment.ple" LineNo="28" ColNo="1">
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
