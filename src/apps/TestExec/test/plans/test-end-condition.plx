<?xml version="1.0" encoding="UTF-8"?>
<!-- test plan -->
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
 xsi:noNamespaceSchemaLocation="http://plexil.svn.sourceforge.net/viewvc/plexil/trunk/schema/core-plexil.xsd">
 <Node NodeType="NodeList">
   <NodeId>Procedure_5_350</NodeId>
   <StartCondition>  
    <EQBoolean>
     <LookupOnChange>
      <Name><StringValue>Event_5520</StringValue></Name>
     </LookupOnChange>
     <BooleanValue>1</BooleanValue>
    </EQBoolean>
   </StartCondition>
   <EndCondition>
     <BooleanValue>1</BooleanValue>
   </EndCondition>
  <NodeBody>
   <NodeList>
    <Node NodeType="Command">
     <NodeId>Step_1_2_3</NodeId>
     <NodeBody>
       <Command>
        <Name><StringValue>blah</StringValue></Name>
       </Command>
     </NodeBody>
    </Node> <!-- end of Procedure_5_350.Step 1_2_3 -->
   </NodeList>
  </NodeBody>
 </Node> <!-- End of node Procedure_5_350 -->
</PlexilPlan>

