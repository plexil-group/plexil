  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="test/if-test.ple">
      <GlobalDeclarations>
          <CommandDeclaration>
              <Name>foo</Name>
          </CommandDeclaration>
          <CommandDeclaration>
              <Name>bar</Name>
              <Parameter Type="Integer">n</Parameter>
          </CommandDeclaration>
      </GlobalDeclarations>
      <If FileName="test/if-test.ple" LineNo="5" ColNo="1">
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>i</Name>
                  <Type>Integer</Type>
                  <InitialValue>
                      <IntegerValue>0</IntegerValue>
                  </InitialValue>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeId>IfTest</NodeId>
          <Condition>
              <GT>
                  <IntegerVariable>i</IntegerVariable>
                  <IntegerValue>0</IntegerValue>
              </GT>
          </Condition>
          <Then>
              <If FileName="test/if-test.ple" LineNo="9" ColNo="3">
                  <NodeId>IfTest__CHILD__1</NodeId>
                  <Condition>
                      <GT>
                          <IntegerVariable>i</IntegerVariable>
                          <IntegerValue>1</IntegerValue>
                      </GT>
                  </Condition>
                  <Then>
                      <NODE FileName="test/if-test.ple" LineNo="12" ColNo="5" NodeType="Command">
                          <NodeId>IfTest__CHILD__1__CHILD__1</NodeId>
                          <NodeBody>
                              <Command>
                                  <Name>
                                      <StringValue>foo</StringValue>
                                  </Name>
                              </Command>
                          </NodeBody>
                      </NODE>
                  </Then>
                  <Else>
                      <NODE FileName="test/if-test.ple" LineNo="16" ColNo="5" NodeType="Command">
                          <NodeId>IfTest__CHILD__1__CHILD__2</NodeId>
                          <NodeBody>
                              <Command>
                                  <Name>
                                      <StringValue>bar</StringValue>
                                  </Name>
                                  <Arguments>
                                      <IntegerVariable>i</IntegerVariable>
                                  </Arguments>
                              </Command>
                          </NodeBody>
                      </NODE>
                  </Else>
              </If>
          </Then>
      </If>
  </PlexilPlan>
