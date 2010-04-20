  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="time-exp-start-cond.ple">
      <Node FileName="time-exp-start-cond.ple" LineNo="2" ColNo="1" NodeType="NodeList">
          <NodeId>test_translator</NodeId>
          <NodeBody>
              <NodeList>
                  <Node FileName="time-exp-start-cond.ple" LineNo="5" ColNo="3" NodeType="Empty">
                      <VariableDeclarations>
                          <DeclareVariable>
                              <Name>now</Name>
                              <Type>Time</Type>
                          </DeclareVariable>
                      </VariableDeclarations>
                      <NodeId>testNode</NodeId>
                      <StartCondition>
                          <EQTime>
                              <TimeVariable>now</TimeVariable>
                              <TimeValue>
                                  <Units>
                                      <IntegerValue>12</IntegerValue>
                                  </Units>
                                  <SubUnits>
                                      <IntegerValue>30</IntegerValue>
                                  </SubUnits>
                              </TimeValue>
                          </EQTime>
                      </StartCondition>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
