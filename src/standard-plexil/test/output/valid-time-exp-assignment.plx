  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="time-exp-assignment.ple">
      <Node FileName="time-exp-assignment.ple" LineNo="2" ColNo="1" NodeType="NodeList">
          <NodeId>test_translator</NodeId>
          <NodeBody>
              <NodeList>
                  <Node FileName="time-exp-assignment.ple" LineNo="5" ColNo="3" NodeType="Assignment">
                      <VariableDeclarations>
                          <DeclareVariable>
                              <Name>currentTime</Name>
                              <Type>Time</Type>
                          </DeclareVariable>
                      </VariableDeclarations>
                      <NodeId>testNode</NodeId>
                      <NodeBody>
                          <Assignment>
                              <TimeVariable>currentTime</TimeVariable>
                              <TimeRHS>
                                  <TimeValue>
                                      <Units>
                                          <IntegerValue>12</IntegerValue>
                                      </Units>
                                      <SubUnits>
                                          <IntegerValue>30</IntegerValue>
                                      </SubUnits>
                                  </TimeValue>
                              </TimeRHS>
                          </Assignment>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
