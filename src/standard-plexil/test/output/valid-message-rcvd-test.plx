<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="Empty" FileName="message-rcvd-test.ple" LineNo="2" ColNo="1">
      <NodeId>MessageReceivedTest</NodeId>
      <StartCondition>
         <LookupOnChange>
            <Name>
               <Concat>
                  <StringValue>MESSAGE__</StringValue>
                  <StringValue>kickme</StringValue>
               </Concat>
            </Name>
         </LookupOnChange>
      </StartCondition>
   </Node>
</PlexilPlan>