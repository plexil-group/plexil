<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <GlobalDeclarations LineNo="1" ColNo="0">
      <CommandDeclaration LineNo="1" ColNo="0">
         <Name>Foo</Name>
         <Parameter>
            <Type>Integer</Type>
         </Parameter>
         <Parameter>
            <Type>String</Type>
         </Parameter>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="Command" LineNo="5" ColNo="2">
      <NodeId>Plan1</NodeId>
      <NodeBody>
         <Command>
            <Name>
               <StringValue>Foo</StringValue>
            </Name>
            <Arguments LineNo="5" ColNo="7">
               <IntegerValue>1</IntegerValue>
               <StringValue>blue</StringValue>
            </Arguments>
         </Command>
      </NodeBody>
   </Node>
</PlexilPlan>