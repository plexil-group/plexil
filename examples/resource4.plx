  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="resource4.ple">
      <Node FileName="resource4.ple" LineNo="1" ColNo="13" NodeType="NodeList">
          <NodeId>SimpleTask</NodeId>
          <NodeBody>
              <NodeList>
                  <Node FileName="resource4.ple" LineNo="3" ColNo="7" NodeType="Command">
                      <VariableDeclarations>
                          <DeclareVariable>
                              <Name>returnValue</Name>
                              <Type>Integer</Type>
                              <InitialValue>
                                  <IntegerValue>-1</IntegerValue>
                              </InitialValue>
                          </DeclareVariable>
                      </VariableDeclarations>
                      <NodeId>C1</NodeId>
                      <EndCondition>
                          <EQNumeric>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <IntegerValue>10</IntegerValue>
                          </EQNumeric>
                      </EndCondition>
                      <PostCondition>
                          <EQInternal>
                              <NodeCommandHandleVariable>
                                  <NodeId>C1</NodeId>
                              </NodeCommandHandleVariable>
                              <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                          </EQInternal>
                      </PostCondition>
                      <NodeBody>
                          <Command>
                              <ResourceList>
                                  <Resource>
                                      <ResourceName>
                                          <StringValue>sys_memory</StringValue>
                                      </ResourceName>
                                      <ResourceLowerBound>
                                          <RealValue>0.8</RealValue>
                                      </ResourceLowerBound>
                                      <ResourceUpperBound>
                                          <RealValue>0.8</RealValue>
                                      </ResourceUpperBound>
                                      <ResourcePriority>
                                          <IntegerValue>20</IntegerValue>
                                      </ResourcePriority>
                                  </Resource>
                              </ResourceList>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <Name>
                                  <StringValue>c1</StringValue>
                              </Name>
                          </Command>
                      </NodeBody>
                  </Node>
                  <Node FileName="resource4.ple" LineNo="13" ColNo="7" NodeType="Command">
                      <VariableDeclarations>
                          <DeclareVariable>
                              <Name>mem_priority</Name>
                              <Type>Integer</Type>
                              <InitialValue>
                                  <IntegerValue>30</IntegerValue>
                              </InitialValue>
                          </DeclareVariable>
                          <DeclareVariable>
                              <Name>returnValue</Name>
                              <Type>Integer</Type>
                              <InitialValue>
                                  <IntegerValue>-1</IntegerValue>
                              </InitialValue>
                          </DeclareVariable>
                      </VariableDeclarations>
                      <NodeId>C2</NodeId>
                      <PostCondition>
                          <EQInternal>
                              <NodeCommandHandleVariable>
                                  <NodeId>C2</NodeId>
                              </NodeCommandHandleVariable>
                              <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                          </EQInternal>
                      </PostCondition>
                      <EndCondition>
                          <EQNumeric>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <IntegerValue>10</IntegerValue>
                          </EQNumeric>
                      </EndCondition>
                      <NodeBody>
                          <Command>
                              <ResourceList>
                                  <Resource>
                                      <ResourceName>
                                          <StringValue>sys_memory</StringValue>
                                      </ResourceName>
                                      <ResourceLowerBound>
                                          <RealValue>0.3</RealValue>
                                      </ResourceLowerBound>
                                      <ResourceUpperBound>
                                          <RealValue>0.3</RealValue>
                                      </ResourceUpperBound>
                                      <ResourcePriority>
                                          <IntegerValue>30</IntegerValue>
                                      </ResourcePriority>
                                  </Resource>
                              </ResourceList>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <Name>
                                  <StringValue>c2</StringValue>
                              </Name>
                          </Command>
                      </NodeBody>
                  </Node>
                  <Node FileName="resource4.ple" LineNo="22" ColNo="7" NodeType="Command">
                      <VariableDeclarations>
                          <DeclareVariable>
                              <Name>returnValue</Name>
                              <Type>Integer</Type>
                              <InitialValue>
                                  <IntegerValue>-1</IntegerValue>
                              </InitialValue>
                          </DeclareVariable>
                      </VariableDeclarations>
                      <NodeId>C3</NodeId>
                      <PostCondition>
                          <EQInternal>
                              <NodeCommandHandleVariable>
                                  <NodeId>C3</NodeId>
                              </NodeCommandHandleVariable>
                              <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                          </EQInternal>
                      </PostCondition>
                      <EndCondition>
                          <EQNumeric>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <IntegerValue>10</IntegerValue>
                          </EQNumeric>
                      </EndCondition>
                      <NodeBody>
                          <Command>
                              <ResourceList>
                                  <Resource>
                                      <ResourceName>
                                          <StringValue>sys_memory</StringValue>
                                      </ResourceName>
                                      <ResourceLowerBound>
                                          <RealValue>-0.1</RealValue>
                                      </ResourceLowerBound>
                                      <ResourceUpperBound>
                                          <RealValue>-0.1</RealValue>
                                      </ResourceUpperBound>
                                      <ResourcePriority>
                                          <IntegerValue>40</IntegerValue>
                                      </ResourcePriority>
                                  </Resource>
                              </ResourceList>
                              <IntegerVariable>returnValue</IntegerVariable>
                              <Name>
                                  <StringValue>c3</StringValue>
                              </Name>
                          </Command>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
