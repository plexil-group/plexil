  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="library-call-test.ple">
      <GlobalDeclarations/>
      <Node FileName="library-call-test.ple" LineNo="4" ColNo="1" NodeType="LibraryNodeCall">
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>k</Name>
                  <Type>Integer</Type>
                  <InitialValue>
                      <IntegerValue>1</IntegerValue>
                  </InitialValue>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeId>LibraryCallTest</NodeId>
          <NodeBody>
              <LibraryNodeCall>
                  <NodeId>LibTest</NodeId>
                  <Alias>
                      <NodeParameter>i</NodeParameter>
                      <IntegerValue>0</IntegerValue>
                  </Alias>
                  <Alias>
                      <NodeParameter>j</NodeParameter>
                      <IntegerVariable>k</IntegerVariable>
                  </Alias>
              </LibraryNodeCall>
          </NodeBody>
      </Node>
  </PlexilPlan>
