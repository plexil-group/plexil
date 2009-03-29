<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList">
      <NodeId>While_d1e3</NodeId>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList">
               <NodeId>ep2cp_WhileBody</NodeId>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="NodeList">
                        <NodeId>ep2cp_WhileTrue</NodeId>
                        <StartCondition>
                           <LT>
                              <IntegerValue>0</IntegerValue>
                              <IntegerValue>1</IntegerValue>
                           </LT>
                        </StartCondition>
                        <RepeatCondition>
                           <LT>
                              <IntegerValue>0</IntegerValue>
                              <IntegerValue>1</IntegerValue>
                           </LT>
                        </RepeatCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Empty">
                                 <NodeId>One</NodeId>
                              </Node>
                           </NodeList>
                        </NodeBody>
                     </Node>
                  </NodeList>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>