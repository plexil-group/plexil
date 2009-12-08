  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="plexil.xsd">
      <Node NodeType="NodeList">
          <NodeId>Wrapper0</NodeId>
          <EndCondition>
              <NEBoolean>
                  <LookupOnChange>
                      <StateName>x</StateName>
                  </LookupOnChange>
                  <BooleanValue>0</BooleanValue>
              </NEBoolean>
          </EndCondition>
          <NodeBody>
              <NodeList>
                  <Node NodeType="NodeList">
                      <NodeId>Wrapper1</NodeId>
                      <NodeBody>
                          <NodeList>
                              <Node NodeType="NodeList">
                                  <NodeId>Wrapper1</NodeId>
                                  <NodeBody>
                                      <NodeList>
                                          <Node NodeType="Command">
                                              <NodeId>FooCall</NodeId>
                                              <RepeatCondition>
                                                  <BooleanValue>true</BooleanValue>
                                              </RepeatCondition>
                                              <NodeBody>
                                                  <Command>
                                                      <CommandName>foo</CommandName>
                                                  </Command>
                                              </NodeBody>
                                          </Node>
                                      </NodeList>
                                  </NodeBody>
                              </Node>
                          </NodeList>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
