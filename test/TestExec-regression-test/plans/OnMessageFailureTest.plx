<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            FileName="plans/OnMessageFailureTest.ple">
   <GlobalDeclarations ColNo="7" LineNo="3">
      <CommandDeclaration ColNo="7" LineNo="3">
         <Name>ReceiveMessage</Name>
         <Return>
            <Name>_return_0</Name>
            <Type>String</Type>
         </Return>
         <Parameter>
            <Name>messageName</Name>
            <Type>String</Type>
         </Parameter>
      </CommandDeclaration>
      <CommandDeclaration ColNo="0" LineNo="4">
         <Name>pprint</Name>
         <AnyParameters/>
      </CommandDeclaration>
   </GlobalDeclarations>
   <Node NodeType="NodeList" epx="Sequence" ColNo="0" LineNo="6">
      <NodeId>OnMessageFailureTest</NodeId>
      <VariableDeclarations>
         <DeclareVariable ColNo="2" LineNo="8">
            <Name>finished</Name>
            <Type>Boolean</Type>
            <InitialValue>
               <BooleanValue>false</BooleanValue>
            </InitialValue>
         </DeclareVariable>
      </VariableDeclarations>
      <InvariantCondition>
         <NoChildFailed>
            <NodeRef dir="self"/>
         </NoChildFailed>
      </InvariantCondition>
      <NodeBody>
         <NodeList>
            <Node NodeType="NodeList" epx="Concurrence" ColNo="2" LineNo="10">
               <NodeId>Concurrence__0</NodeId>
               <NodeBody>
                  <NodeList>
                     <Node NodeType="NodeList" epx="CheckedSequence" ColNo="4" LineNo="12">
                        <NodeId>OnMessage__1</NodeId>
                        <VariableDeclarations>
                           <DeclareVariable>
                              <Name>ep2cp_hdl</Name>
                              <Type>String</Type>
                           </DeclareVariable>
                        </VariableDeclarations>
                        <InvariantCondition>
                           <NoChildFailed>
                              <NodeRef dir="self"/>
                           </NoChildFailed>
                        </InvariantCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Command" epx="aux">
                                 <NodeId generated="1">ep2cp_OnMessage_MsgWait</NodeId>
                                 <EndCondition>
                                    <IsKnown>
                                       <StringVariable>ep2cp_hdl</StringVariable>
                                    </IsKnown>
                                 </EndCondition>
                                 <PostCondition>
                                    <EQInternal>
                                       <NodeCommandHandleVariable>
                                          <NodeRef dir="self"/>
                                       </NodeCommandHandleVariable>
                                       <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                                    </EQInternal>
                                 </PostCondition>
                                 <NodeBody>
                                    <Command>
                                       <StringVariable>ep2cp_hdl</StringVariable>
                                       <Name>
                                          <StringValue>ReceiveMessage</StringValue>
                                       </Name>
                                       <Arguments>
                                          <StringValue>Quit</StringValue>
                                       </Arguments>
                                    </Command>
                                 </NodeBody>
                              </Node>
                              <Node ColNo="21" LineNo="12" NodeType="NodeList" epx="Sequence">
                                 <NodeId>BLOCK__2</NodeId>
                                 <StartCondition>
                                    <Finished>
                                       <NodeRef dir="sibling">ep2cp_OnMessage_MsgWait</NodeRef>
                                    </Finished>
                                 </StartCondition>
                                 <InvariantCondition>
                                    <NoChildFailed>
                                       <NodeRef dir="self"/>
                                    </NoChildFailed>
                                 </InvariantCondition>
                                 <NodeBody>
                                    <NodeList>
                                       <Node ColNo="6" LineNo="13" NodeType="Command">
                                          <NodeId>COMMAND__3</NodeId>
                                          <NodeBody>
                                             <Command ColNo="6" LineNo="13">
                                                <Name>
                                                   <StringValue>pprint</StringValue>
                                                </Name>
                                                <Arguments ColNo="13" LineNo="13">
                                                   <StringValue>OnMessageFailureTest exiting on Quit message</StringValue>
                                                </Arguments>
                                             </Command>
                                          </NodeBody>
                                       </Node>
                                       <Node ColNo="6" LineNo="14" NodeType="Assignment">
                                          <NodeId>ASSIGNMENT__4</NodeId>
                                          <StartCondition>
                                             <Finished>
                                                <NodeRef dir="sibling">COMMAND__3</NodeRef>
                                             </Finished>
                                          </StartCondition>
                                          <NodeBody>
                                             <Assignment ColNo="6" LineNo="14">
                                                <BooleanVariable>finished</BooleanVariable>
                                                <BooleanRHS>
                                                   <BooleanValue>true</BooleanValue>
                                                </BooleanRHS>
                                             </Assignment>
                                          </NodeBody>
                                       </Node>
                                    </NodeList>
                                 </NodeBody>
                              </Node>
                           </NodeList>
                        </NodeBody>
                     </Node>
                     <Node NodeType="NodeList" epx="While" ColNo="4" LineNo="17">
                        <NodeId>BLOCK__5</NodeId>
                        <ExitCondition ColNo="6" LineNo="18">
                           <BooleanVariable>finished</BooleanVariable>
                        </ExitCondition>
                        <RepeatCondition>
                           <Succeeded>
                              <NodeRef dir="child">ep2cp_WhileTest</NodeRef>
                           </Succeeded>
                        </RepeatCondition>
                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Empty" epx="Condition">
                                 <NodeId generated="1">ep2cp_WhileTest</NodeId>
                                 <PostCondition>
                                    <NOT ColNo="13" LineNo="20">
                                       <BooleanVariable>finished</BooleanVariable>
                                    </NOT>
                                 </PostCondition>
                              </Node>
                              <Node NodeType="NodeList" epx="Action" ColNo="8" LineNo="24">
                                 <NodeId>Concurrence__7</NodeId>
                                 <PostCondition ColNo="10" LineNo="25">
                                    <OR ColNo="12" LineNo="26">
                                       <Succeeded ColNo="24" LineNo="25">
                                          <NodeRef dir="child">SimpleOnMessage</NodeRef>
                                       </Succeeded>
                                       <AND>
                                          <Finished>
                                             <NodeRef dir="child">SimpleOnMessage</NodeRef>
                                          </Finished>
                                          <EQInternal>
                                             <NodeFailureVariable>
                                                <NodeRef dir="child">SimpleOnMessage</NodeRef>
                                             </NodeFailureVariable>
                                             <NodeFailureValue>INVARIANT_CONDITION_FAILED</NodeFailureValue>
                                          </EQInternal>
                                       </AND>
                                    </OR>
                                 </PostCondition>
                                 <StartCondition>
                                    <Succeeded>
                                       <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
                                    </Succeeded>
                                 </StartCondition>
                                 <SkipCondition>
                                    <PostconditionFailed>
                                       <NodeRef dir="sibling">ep2cp_WhileTest</NodeRef>
                                    </PostconditionFailed>
                                 </SkipCondition>
                                 <NodeBody>
                                    <NodeList>
                                       <Node NodeType="NodeList" epx="CheckedSequence" ColNo="8" LineNo="28">
                                          <NodeId>SimpleOnMessage</NodeId>
                                          <VariableDeclarations>
                                             <DeclareVariable>
                                                <Name>ep2cp_hdl</Name>
                                                <Type>String</Type>
                                             </DeclareVariable>
                                          </VariableDeclarations>
                                          <InvariantCondition>
                                             <NoChildFailed>
                                                <NodeRef dir="self"/>
                                             </NoChildFailed>
                                          </InvariantCondition>
                                          <NodeBody>
                                             <NodeList>
                                                <Node NodeType="Command" epx="aux">
                                                   <NodeId generated="1">ep2cp_OnMessage_MsgWait</NodeId>
                                                   <EndCondition>
                                                      <IsKnown>
                                                         <StringVariable>ep2cp_hdl</StringVariable>
                                                      </IsKnown>
                                                   </EndCondition>
                                                   <PostCondition>
                                                      <EQInternal>
                                                         <NodeCommandHandleVariable>
                                                            <NodeRef dir="self"/>
                                                         </NodeCommandHandleVariable>
                                                         <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                                                      </EQInternal>
                                                   </PostCondition>
                                                   <NodeBody>
                                                      <Command>
                                                         <StringVariable>ep2cp_hdl</StringVariable>
                                                         <Name>
                                                            <StringValue>ReceiveMessage</StringValue>
                                                         </Name>
                                                         <Arguments>
                                                            <StringValue>Foo</StringValue>
                                                         </Arguments>
                                                      </Command>
                                                   </NodeBody>
                                                </Node>
                                                <Node ColNo="12" LineNo="30" NodeType="Command">
                                                   <NodeId>COMMAND__8</NodeId>
                                                   <StartCondition>
                                                      <Finished>
                                                         <NodeRef dir="sibling">ep2cp_OnMessage_MsgWait</NodeRef>
                                                      </Finished>
                                                   </StartCondition>
                                                   <NodeBody>
                                                      <Command ColNo="12" LineNo="30">
                                                         <Name>
                                                            <StringValue>pprint</StringValue>
                                                         </Name>
                                                         <Arguments ColNo="19" LineNo="30">
                                                            <StringValue>Foo!</StringValue>
                                                         </Arguments>
                                                      </Command>
                                                   </NodeBody>
                                                </Node>
                                             </NodeList>
                                          </NodeBody>
                                       </Node>
                                       <Node ColNo="10" LineNo="32" NodeType="Command">
                                          <NodeId>BLOCK__9</NodeId>
                                          <StartCondition ColNo="12" LineNo="33">
                                             <AND>
                                                <Finished>
                                                   <NodeRef dir="sibling">SimpleOnMessage</NodeRef>
                                                </Finished>
                                                <EQInternal>
                                                   <NodeFailureVariable>
                                                      <NodeRef dir="sibling">SimpleOnMessage</NodeRef>
                                                   </NodeFailureVariable>
                                                   <NodeFailureValue>INVARIANT_CONDITION_FAILED</NodeFailureValue>
                                                </EQInternal>
                                             </AND>
                                          </StartCondition>
                                          <SkipCondition ColNo="12" LineNo="34">
                                             <Succeeded ColNo="26" LineNo="34">
                                                <NodeRef dir="sibling">SimpleOnMessage</NodeRef>
                                             </Succeeded>
                                          </SkipCondition>
                                          <NodeBody>
                                             <Command ColNo="12" LineNo="35">
                                                <Name>
                                                   <StringValue>pprint</StringValue>
                                                </Name>
                                                <Arguments ColNo="19" LineNo="35">
                                                   <StringValue>SimpleOnMessage failed</StringValue>
                                                </Arguments>
                                             </Command>
                                          </NodeBody>
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
            <Node ColNo="2" LineNo="41" NodeType="Command">
               <NodeId>COMMAND__11</NodeId>
               <StartCondition>
                  <Finished>
                     <NodeRef dir="sibling">Concurrence__0</NodeRef>
                  </Finished>
               </StartCondition>
               <NodeBody>
                  <Command ColNo="2" LineNo="41">
                     <Name>
                        <StringValue>pprint</StringValue>
                     </Name>
                     <Arguments ColNo="9" LineNo="41">
                        <StringValue>OnMessageFailureTest complete</StringValue>
                     </Arguments>
                  </Command>
               </NodeBody>
            </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
