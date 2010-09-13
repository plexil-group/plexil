  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd" FileName="lookup-exp-as-start-cond.ple">
      <GlobalDeclarations>
          <StateDeclaration>
              <Name>LAPR02FC0617U</Name>
              <Return Type="Real">_State_return_1</Return>
          </StateDeclaration>
      </GlobalDeclarations>
      <Node FileName="lookup-exp-as-start-cond.ple" LineNo="4" ColNo="1" NodeType="Empty">
          <VariableDeclarations>
              <DeclareVariable>
                  <Name>x</Name>
                  <Type>Real</Type>
              </DeclareVariable>
          </VariableDeclarations>
          <NodeId>testNode</NodeId>
          <StartCondition>
              <EQNumeric>
                  <LookupOnChange>
                      <Name>
                          <StringValue>LAPR02FC0617U</StringValue>
                      </Name>
                  </LookupOnChange>
                  <RealVariable>x</RealVariable>
              </EQNumeric>
          </StartCondition>
      </Node>
  </PlexilPlan>
