  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="plexil.xsd">
      <GlobalDeclarations/>
      <Node NodeType="NodeList">
          <Interface>
              <In>
                <DeclareVariable>
                  <Name>lr</Name>
                  <Type>Real</Type>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>li</Name>
                  <Type>Integer</Type>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>ls</Name>
                  <Type>String</Type>
                </DeclareVariable>
                <DeclareVariable>
                  <Name>lb</Name>
                  <Type>Boolean</Type>
                </DeclareVariable>

<!--                   <RealVariable>lr</RealVariable> -->
<!--                   <IntegerVariable>li</IntegerVariable> -->
<!--                   <StringVariable>ls</StringVariable> -->
<!--                   <BooleanVariable>lb</BooleanVariable> -->
              </In>
          </Interface>
          <NodeId>LibTest1</NodeId>
          <NodeBody>
              <NodeList>
                  <Node NodeType="Command">
                      <NodeId>FooCall1</NodeId>
                      <NodeBody>
                          <Command>
                              <CommandName>foo</CommandName>
                              <Arguments>
                                  <BooleanVariable>lb</BooleanVariable>
                                  <IntegerVariable>li</IntegerVariable>
                                  <RealVariable>lr</RealVariable>
                                  <StringVariable>ls</StringVariable>
                              </Arguments>
                          </Command>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
