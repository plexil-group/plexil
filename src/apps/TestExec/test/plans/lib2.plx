<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations LineNo="1" ColNo="0">
      <LibraryNodeDeclaration LineNo="1" ColNo="0">
         <Name>LibTest1</Name>
         <Interface LineNo="1" ColNo="24">
            <In>
               <DeclareVariable LineNo="1" ColNo="24">
                  <Name>lb</Name>
                  <Type>Boolean</Type>
               </DeclareVariable>
               <DeclareVariable LineNo="2" ColNo="24">
                  <Name>li</Name>
                  <Type>Integer</Type>
               </DeclareVariable>
               <DeclareVariable LineNo="3" ColNo="24">
                  <Name>lr</Name>
                  <Type>Real</Type>
               </DeclareVariable>
               <DeclareVariable LineNo="4" ColNo="24">
                  <Name>ls</Name>
                  <Type>String</Type>
               </DeclareVariable>
            </In>
         </Interface>
      </LibraryNodeDeclaration>
      <LibraryNodeDeclaration LineNo="5" ColNo="0">
         <Name>LibTest2</Name>
         <Interface LineNo="5" ColNo="24">
            <In>
               <DeclareVariable LineNo="5" ColNo="24">
                  <Name>y</Name>
                  <Type>Real</Type>
               </DeclareVariable>
               <DeclareVariable LineNo="5" ColNo="35">
                  <Name>z</Name>
                  <Type>Real</Type>
               </DeclareVariable>
            </In>
         </Interface>
      </LibraryNodeDeclaration>
      <CommandDeclaration LineNo="6" ColNo="0">
         <Name>bar</Name>
         <Parameter>
            <Type>Real</Type>
         </Parameter>
         <Parameter>
            <Type>Real</Type>
         </Parameter>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Sequence" LineNo="10" ColNo="2">
      <NodeId>LibTest2</NodeId>
      <Interface>
         <In>
            <DeclareVariable LineNo="10" ColNo="5">
               <Name>y</Name>
               <Type>Real</Type>
            </DeclareVariable>
            <DeclareVariable LineNo="11" ColNo="5">
               <Name>z</Name>
               <Type>Real</Type>
            </DeclareVariable>
         </In>
      </Interface>
      <InvariantCondition>
         <AND>
            <NOT>
               <OR>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>BarCall2</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
                  <EQInternal>
                     <NodeOutcomeVariable>
                        <NodeId>Call2LibTest1</NodeId>
                     </NodeOutcomeVariable>
                     <NodeOutcomeValue>FAILURE</NodeOutcomeValue>
                  </EQInternal>
               </OR>
            </NOT>
         </AND>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="Command" LineNo="13" ColNo="12">
               <NodeId>BarCall2</NodeId>
               <NodeBody>
                  <Command>
                     <Name>
                        <StringValue>bar</StringValue>
                     </Name>
                     <Arguments LineNo="13" ColNo="16">
                        <RealVariable>y</RealVariable>
                        <RealVariable>z</RealVariable>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
            <Node NodeType="LibraryNodeCall">
               <NodeId>Call2LibTest1</NodeId>
               <StartCondition>
                  <AND>
                     <EQInternal>
                        <NodeStateVariable>
                           <NodeId>BarCall2</NodeId>
                        </NodeStateVariable>
                        <NodeStateValue>FINISHED</NodeStateValue>
                     </EQInternal>
                  </AND>
               </StartCondition>
               <NodeBody>
                  <LibraryNodeCall>
                     <NodeId>LibTest1</NodeId>
                     <Alias>
                        <NodeParameter>lb</NodeParameter>
                        <BooleanValue>false</BooleanValue>
                     </Alias>
                     <Alias>
                        <NodeParameter>li</NodeParameter>
                        <IntegerValue>123</IntegerValue>
                     </Alias>
                     <Alias>
                        <NodeParameter>lr</NodeParameter>
                        <RealVariable>y</RealVariable>
                     </Alias>
                     <Alias>
                        <NodeParameter>ls</NodeParameter>
                        <StringValue>what!</StringValue>
                     </Alias>
                  </LibraryNodeCall>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>