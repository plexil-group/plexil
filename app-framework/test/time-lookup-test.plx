  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/supported-plexil.xsd" FileName="../app-framework/test/time-lookup-test.ple">
      <GlobalDeclarations>
          <StateDeclaration>
              <Name>time</Name>
              <Return Type="Real">_State_return_1</Return>
          </StateDeclaration>
      </GlobalDeclarations>
      <Node NodeType="Empty" FileName="../app-framework/test/time-lookup-test.ple" LineNo="4" ColNo="1">
          <NodeId>time_lookup_test</NodeId>
          <EndCondition>
              <GE>
                  <LookupOnChange>
                      <Name>
                          <StringValue>time</StringValue>
                      </Name>
                      <Tolerance>
                          <RealValue>1</RealValue>
                      </Tolerance>
                  </LookupOnChange>
                  <ADD>
                      <NodeTimepointValue>
                          <NodeId>time_lookup_test</NodeId>
                          <NodeStateValue>EXECUTING</NodeStateValue>
                          <Timepoint>START</Timepoint>
                      </NodeTimepointValue>
                      <IntegerValue>5</IntegerValue>
                  </ADD>
              </GE>
          </EndCondition>
      </Node>
  </PlexilPlan>
