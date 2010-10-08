  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd" FileName="numeric-exp-test.ple">
      <Node FileName="numeric-exp-test.ple" LineNo="1" ColNo="1" NodeType="Assignment">
          <NodeId>_GLOBAL_CONTEXT___CHILD__1</NodeId>
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>foo</Name>
                  <Type>Integer</Type>
                  <InitialValue>
                      <IntegerValue>0</IntegerValue>
                  </InitialValue>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeBody>
              <Assignment>
                  <IntegerVariable>foo</IntegerVariable>
                  <NumericRHS>
                      <IntegerValue>2</IntegerValue>
                  </NumericRHS>
              </Assignment>
          </NodeBody>
      </Node>
  </PlexilPlan>
