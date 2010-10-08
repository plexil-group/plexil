  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd" FileName="string-test.ple">
      <Node FileName="string-test.ple" LineNo="2" ColNo="1" NodeType="NodeList">
          <NodeId>StringTest</NodeId>
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>foo</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>basic string</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>bar</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with a newline
</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>baz</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with 3 char octal escape&#xff;</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>bez</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with 2 char octal escape &#x1d;</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>boz</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with 2 char octal escape = and some more stuff</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>buz</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with 1 char octal escape&#x0; and some more stuff</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>biz</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with 1 char octal escape&#x4;</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>bletch</Name>
                  <Type>String</Type>
                  <InitialValue>
                      <StringValue>string with a Unicode escape
 followed by text</StringValue>
                  </InitialValue>
              </DeclareVariable>
              <DeclareArray>
                  <Name>ari</Name>
                  <Type>String</Type>
                  <MaxSize>3</MaxSize>
                  <InitialValue>
                      <StringValue>zero</StringValue>
                      <StringValue>one</StringValue>
                      <StringValue>two</StringValue>
                  </InitialValue>
              </DeclareArray>
              <DeclareVariable>
                  <Name>fubar</Name>
                  <Type>String</Type>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>bazbezboz</Name>
                  <Type>String</Type>
              </DeclareVariable>
              <DeclareVariable>
                  <Name>junk</Name>
                  <Type>String</Type>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeBody>
              <NodeList>
                  <Node FileName="string-test.ple" LineNo="16" ColNo="4" NodeType="Assignment">
                      <NodeId>StringTest__CHILD__1</NodeId>
                      <NodeBody>
                          <Assignment>
                              <StringVariable>fubar</StringVariable>
                              <StringRHS>
                                  <Concat>
                                      <StringVariable>foo</StringVariable>
                                      <StringVariable>bar</StringVariable>
                                  </Concat>
                              </StringRHS>
                          </Assignment>
                      </NodeBody>
                  </Node>
                  <Node FileName="string-test.ple" LineNo="18" ColNo="4" NodeType="Assignment">
                      <NodeId>StringTest__CHILD__2</NodeId>
                      <NodeBody>
                          <Assignment>
                              <StringVariable>bazbezboz</StringVariable>
                              <StringRHS>
                                  <Concat>
                                      <Concat>
                                          <StringVariable>baz</StringVariable>
                                          <StringVariable>bez</StringVariable>
                                      </Concat>
                                      <StringVariable>boz</StringVariable>
                                  </Concat>
                              </StringRHS>
                          </Assignment>
                      </NodeBody>
                  </Node>
                  <Node FileName="string-test.ple" LineNo="20" ColNo="4" NodeType="Assignment">
                      <NodeId>StringTest__CHILD__3</NodeId>
                      <NodeBody>
                          <Assignment>
                              <StringVariable>junk</StringVariable>
                              <StringRHS>
                                  <Concat>
                                      <Concat>
                                          <StringVariable>foo</StringVariable>
                                          <StringValue> </StringValue>
                                      </Concat>
                                      <ArrayElement>
                                          <Name>ari</Name>
                                          <Index>
                                              <IntegerValue>0</IntegerValue>
                                          </Index>
                                      </ArrayElement>
                                  </Concat>
                              </StringRHS>
                          </Assignment>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
