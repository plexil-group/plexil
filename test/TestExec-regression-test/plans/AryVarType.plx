<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
             FileName="examples/broken/AryVarType.ple">
   <GlobalDeclarations ColNo="0" LineNo="1">
      <CommandDeclaration ColNo="0" LineNo="1">
         <Name>pprint</Name>
         <AnyParameters/>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node ColNo="0" LineNo="3" NodeType="Command">
      <NodeId>AryVarType</NodeId>
      <VariableDeclarations>
         <DeclareArray ColNo="4" LineNo="5">
            <Name>ra</Name>
            <Type>Real</Type>
            <MaxSize>6</MaxSize>
         </DeclareArray>
      </VariableDeclarations>
      <NodeBody>
         <Command ColNo="4" LineNo="6">
            <Name>
               <StringValue>pprint</StringValue>
            </Name>
            <Arguments ColNo="11" LineNo="6">
               <ANY_KNOWN ColNo="11" LineNo="6">
                  <ArrayVariable>ra</ArrayVariable>
               </ANY_KNOWN>
            </Arguments>
         </Command>
      </NodeBody>
   </Node>
</PlexilPlan>
