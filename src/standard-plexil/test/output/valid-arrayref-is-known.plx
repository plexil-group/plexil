  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="arrayref-is-known.ple">
      <Node FileName="arrayref-is-known.ple" LineNo="4" ColNo="1" NodeType="NodeList">
          <NodeId>Root</NodeId>
          <NodeBody>
              <NodeList>
                  <Node FileName="arrayref-is-known.ple" LineNo="7" ColNo="5" NodeType="Empty">
                      <VariableDeclarations>
                          <DeclareArray>
                              <Name>Numbers</Name>
                              <Type>Integer</Type>
                              <MaxSize>3</MaxSize>
                          </DeclareArray>
                      </VariableDeclarations>
                      <NodeId>One</NodeId>
                      <PostCondition>
                          <IsKnown>
                              <ArrayVariable>Numbers</ArrayVariable>
                          </IsKnown>
                      </PostCondition>
                  </Node>
                  <Node FileName="arrayref-is-known.ple" LineNo="13" ColNo="5" NodeType="Empty">
                      <VariableDeclarations>
                          <DeclareArray>
                              <Name>Numbers</Name>
                              <Type>Integer</Type>
                              <MaxSize>3</MaxSize>
                          </DeclareArray>
                      </VariableDeclarations>
                      <NodeId>Two</NodeId>
                      <PostCondition>
                          <IsKnown>
                              <ArrayElement>
                                  <Name>Numbers</Name>
                                  <Index>
                                      <IntegerValue>0</IntegerValue>
                                  </Index>
                              </ArrayElement>
                          </IsKnown>
                      </PostCondition>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
