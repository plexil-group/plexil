  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd" FileName="failure.ple">
      <Node FileName="failure.ple" LineNo="2" ColNo="1" NodeType="NodeList">
          <NodeId>root</NodeId>
          <EndCondition>
              <EQInternal>
                  <NodeFailureVariable>
                      <NodeRef>foo</NodeRef>
                  </NodeFailureVariable>
                  <NodeFailureValue>INFINITE_LOOP</NodeFailureValue>
              </EQInternal>
          </EndCondition>
          <NodeBody>
              <NodeList>
                  <Node FileName="failure.ple" LineNo="6" ColNo="3" NodeType="Empty">
                      <NodeId>foo</NodeId>
                  </Node>
              </NodeList>
          </NodeBody>
      </Node>
  </PlexilPlan>
