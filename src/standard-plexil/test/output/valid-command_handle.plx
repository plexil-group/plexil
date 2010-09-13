  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd" FileName="command_handle.ple">
      <Node FileName="command_handle.ple" LineNo="2" ColNo="1" NodeType="NodeList">
          <NodeId>root</NodeId>
          <EndCondition>
              <EQInternal>
                  <NodeCommandHandleVariable>
                      <NodeId>foo</NodeId>
                  </NodeCommandHandleVariable>
                  <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
              </EQInternal>
          </EndCondition>
          <NodeBody>
              <NodeList>
                  <Node FileName="command_handle.ple" LineNo="6" ColNo="3" NodeType="Command">
                      <NodeId>foo</NodeId>
                      <NodeBody>
                          <Command>
                              <Name>
                                  <StringValue>foo</StringValue>
                              </Name>
                          </Command>
                      </NodeBody>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
