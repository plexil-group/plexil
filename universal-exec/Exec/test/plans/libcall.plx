  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="plexil.xsd">
      <GlobalDeclarations/>
      <Node NodeType="NodeList">
          <VariableDeclarations>
                <DeclareVariable>
                  <Name>i</Name>
                  <Type>Integer</Type>
                  <InitialValue>
                    <IntegerValue>1966</IntegerValue>
                  </InitialValue>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>r</Name>
                  <Type>Real</Type>
                  <InitialValue>
                    <RealValue>3.1415</RealValue>
                  </InitialValue>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>b</Name>
                  <Type>Boolean</Type>
                  <InitialValue>
                    <BooleanValue>true</BooleanValue>
                  </InitialValue>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>s</Name>
                  <Type>String</Type>
                  <InitialValue>
                    <StringValue>"hello"</StringValue>
                  </InitialValue>
                </DeclareVariable>

<!--               <DeclareInteger> -->
<!--                   <IntegerVariable>i</IntegerVariable> -->
<!--                   <IntegerValue>1966</IntegerValue> -->
<!--               </DeclareInteger> -->
<!--               <DeclareReal> -->
<!--                   <RealVariable>r</RealVariable> -->
<!--                   <RealValue>3.1415</RealValue> -->
<!--               </DeclareReal> -->
<!--               <DeclareBoolean> -->
<!--                   <BooleanVariable>b</BooleanVariable> -->
<!--                   <BooleanValue>true</BooleanValue> -->
<!--               </DeclareBoolean> -->
<!--               <DeclareString> -->
<!--                   <StringVariable>s</StringVariable> -->
<!--                   <StringValue>&quot;hello&quot;</StringValue> -->
<!--               </DeclareString> -->
          </VariableDeclarations>
          <NodeId>List</NodeId>
          <NodeBody>
              <NodeList>
                  <Node NodeType="LibraryNodeCall">
                      <NodeId>CallLibTest1</NodeId>
                      <StartCondition>
                          <EQInternal>
                              <NodeOutcomeVariable>
                                  <NodeId>CallLibTest2</NodeId>
                              </NodeOutcomeVariable>
                              <NodeOutcomeValue>SUCCESS</NodeOutcomeValue>
                          </EQInternal>
                      </StartCondition>
                      <NodeBody>
                          <LibraryNodeCall>
                              <NodeId>LibTest1</NodeId>
                              <Alias>
                                  <NodeParameter>lb</NodeParameter>
                                  <BooleanVariable>b</BooleanVariable>
                              </Alias>
                              <Alias>
                                  <NodeParameter>li</NodeParameter>
                                  <IntegerVariable>i</IntegerVariable>
                              </Alias>
                              <Alias>
                                  <NodeParameter>lr</NodeParameter>
                                  <RealVariable>r</RealVariable>
                              </Alias>
                              <Alias>
                                  <NodeParameter>ls</NodeParameter>
                                  <StringVariable>s</StringVariable>
                              </Alias>
                          </LibraryNodeCall>
                      </NodeBody>
                  </Node>
                  <Node NodeType="LibraryNodeCall">
                      <NodeId>CallLibTest2</NodeId>
                      <NodeBody>
                          <LibraryNodeCall>
                              <NodeId>LibTest2</NodeId>
                              <Alias>
                                  <NodeParameter>y</NodeParameter>
                                  <RealVariable>r</RealVariable>
                              </Alias>
<!--                               <Alias> -->
<!--                                   <NodeParameter>z</NodeParameter> -->
<!--                                   <RealVariable>r</RealVariable> -->
<!--                               </Alias> -->
                          </LibraryNodeCall>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
