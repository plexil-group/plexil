<?xml version="1.0" encoding="UTF-8"?><!-- Generated by PlexiLisp --><PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:tr="extended-plexil-translator"><GlobalDeclarations>
    <CommandDeclaration>
      <Name>get_next_inspection_point</Name>
      <Return>
        <Type>Integer</Type>
      </Return>
    </CommandDeclaration>
    <CommandDeclaration>
      <Name>move_to_waypoint</Name>
      <Return>
        <Type>Integer</Type>
      </Return>
      <Parameter>
        <Type>String</Type>
      </Parameter>
      <Parameter>
        <Type>String</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
      <Parameter>
        <Type>Real</Type>
      </Parameter>
    </CommandDeclaration>
    <CommandDeclaration>
      <Name>acquire</Name>
      <Return>
        <Type>Integer</Type>
      </Return>
    </CommandDeclaration>
    <StateDeclaration>
      <Name>name</Name>
      <Return>
        <Type>String</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>frame</Name>
      <Return>
        <Type>String</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>waypoint_x</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>waypoint_y</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>waypoint_z</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>waypoint_a</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>isDirectional</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>tolerance</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>headingTolerance</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>hintedSpeed</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>inspect_waypoint</Name>
      <Return>
        <Type>Integer</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>task_id</Name>
      <Return>
        <Type>Integer</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>timeout</Name>
      <Return>
        <Type>Real</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>plan_paused</Name>
      <Return>
        <Type>Boolean</Type>
      </Return>
    </StateDeclaration>
    <StateDeclaration>
      <Name>abort_plan</Name>
      <Return>
        <Type>Boolean</Type>
      </Return>
    </StateDeclaration>
  </GlobalDeclarations><Node NodeType="NodeList"><NodeId>SiteSurveyWithEOF</NodeId><VariableDeclarations>
      <DeclareVariable>
        <Name>task_type</Name>
        <Type>Integer</Type>
      </DeclareVariable>
      <DeclareVariable>
        <Name>is_aborted</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>false</BooleanValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>abort_due_to_exception</Name>
        <Type>Boolean</Type>
        <InitialValue>
          <BooleanValue>false</BooleanValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>waypt_id</Name>
        <Type>Integer</Type>
      </DeclareVariable>
    </VariableDeclarations><EndCondition><OR><Finished>
          <NodeId>SignalEndOfPlan</NodeId>
        </Finished><Finished>
          <NodeId>SendAbortUpdate</NodeId>
        </Finished><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>abort_due_to_exception</BooleanVariable></EQBoolean></OR></EndCondition><NodeBody>
      <NodeList>
        <Node NodeType="NodeList"><NodeId>SiteSurveyWrapper</NodeId><VariableDeclarations>
            <DeclareVariable>
              <Name>task_type</Name>
              <Type>Integer</Type>
            </DeclareVariable>
            <DeclareVariable>
              <Name>is_paused</Name>
              <Type>Boolean</Type>
              <InitialValue>
                <BooleanValue>false</BooleanValue>
              </InitialValue>
            </DeclareVariable>
          </VariableDeclarations><EndCondition><OR><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>task_type</IntegerVariable></EQNumeric><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>is_aborted</BooleanVariable></EQBoolean><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>abort_due_to_exception</BooleanVariable></EQBoolean></OR></EndCondition><NodeBody>
            <NodeList>
              <Node NodeType="NodeList"><NodeId>SiteSurvey</NodeId><VariableDeclarations>
                  <DeclareVariable>
                    <Name>task_result_str</Name>
                    <Type>String</Type>
                  </DeclareVariable>
                  <DeclareVariable>
                    <Name>cmd_return_val</Name>
                    <Type>Integer</Type>
                  </DeclareVariable>
                  <DeclareVariable>
                    <Name>cmd_name</Name>
                    <Type>String</Type>
                    <InitialValue>
<StringValue>nil</StringValue>
                    </InitialValue>
                  </DeclareVariable>
                </VariableDeclarations><RepeatCondition><NENumeric><IntegerValue>0</IntegerValue><IntegerVariable>task_type</IntegerVariable></NENumeric></RepeatCondition><NodeBody>
                  <NodeList>
                    <Node NodeType="NodeList"><NodeId>Loop</NodeId><StartCondition><EQBoolean><BooleanValue>0</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean></StartCondition><EndCondition><Finished>
                          <NodeId>CheckIfAbortPlan</NodeId>
                        </Finished></EndCondition><NodeBody>
                        <NodeList>
                          <Node NodeType="Command"><NodeId>RequestNextWaypt</NodeId><VariableDeclarations>
                              <DeclareVariable>
                                <Name>ret_val</Name>
                                <Type>Integer</Type>
                              </DeclareVariable>
                            </VariableDeclarations><StartCondition><EQBoolean><BooleanValue>0</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean></StartCondition><PostCondition><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>ret_val</IntegerVariable></EQNumeric></PostCondition><EndCondition><IsKnown><IntegerVariable>ret_val</IntegerVariable></IsKnown></EndCondition><NodeBody>
                              <Command>
                                <IntegerVariable>ret_val</IntegerVariable>
                                <Name>
<StringValue>get_next_inspection_point</StringValue>
                                </Name>
                              </Command>
                            </NodeBody></Node>
                          <Node NodeType="Assignment"><NodeId>SetWaypointRequestFailed</NodeId><StartCondition><Failed>
                                <NodeId>RequestNextWaypt</NodeId>
                              </Failed></StartCondition><NodeBody>
                              <Assignment>
                                <StringVariable>cmd_name</StringVariable>
                                <StringRHS>
<StringValue>get_next_inspection_point</StringValue>
                                </StringRHS>
                              </Assignment>
                            </NodeBody></Node>
                          <Node NodeType="NodeList"><NodeId>GetAndProcessWaypoint</NodeId><VariableDeclarations>
                              <DeclareVariable>
                                <Name>waypt_name</Name>
                                <Type>String</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_frame</Name>
                                <Type>String</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_x</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_y</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_z</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_a</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_is_directional</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_tolerance</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_heading_tolerance</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_hinted_speed</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                              <DeclareVariable>
                                <Name>waypt_timeout</Name>
                                <Type>Real</Type>
                              </DeclareVariable>
                            </VariableDeclarations><StartCondition><AND><EQBoolean><BooleanValue>0</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean><Finished>
                                  <NodeId>RequestNextWaypt</NodeId>
                                </Finished></AND></StartCondition><PreCondition><Succeeded>
                                <NodeId>RequestNextWaypt</NodeId>
                              </Succeeded></PreCondition><InvariantCondition><EQBoolean><BooleanValue>0</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean></InvariantCondition><EndCondition><OR><Failed>
                                  <NodeId>GetNewWaypt</NodeId>
                                </Failed><Finished>
                                  <NodeId>DriveAndAcquire</NodeId>
                                </Finished></OR></EndCondition><NodeBody>
                              <NodeList>
                                <Node NodeType="NodeList"><NodeId>GetNewWaypt</NodeId><PostCondition><AND><Succeeded>
                                        <NodeId>Get_x</NodeId>
                                      </Succeeded><Succeeded>
                                        <NodeId>Get_y</NodeId>
                                      </Succeeded><Succeeded>
                                        <NodeId>Get_task_type</NodeId>
                                      </Succeeded><Succeeded>
                                        <NodeId>Get_task_id</NodeId>
                                      </Succeeded></AND></PostCondition><NodeBody>
                                    <NodeList>
                                      <Node NodeType="Assignment"><NodeId>Get_name</NodeId><NodeBody>
                                          <Assignment>
                                            <StringVariable>waypt_name</StringVariable>
                                            <StringRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>name</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </StringRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_frame</NodeId><NodeBody>
                                          <Assignment>
                                            <StringVariable>waypt_frame</StringVariable>
                                            <StringRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>frame</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </StringRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_x</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_x</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>waypoint_x</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_y</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_y</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>waypoint_y</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_z</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_z</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>waypoint_z</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_a</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_a</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>waypoint_a</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_dir</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_is_directional</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>isDirectional</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_tolerance</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_tolerance</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>tolerance</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_heading_tolerance</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_heading_tolerance</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>headingTolerance</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_hinted_speed</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_hinted_speed</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>hintedSpeed</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_task_type</NodeId><NodeBody>
                                          <Assignment>
                                            <IntegerVariable>task_type</IntegerVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>inspect_waypoint</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_task_id</NodeId><NodeBody>
                                          <Assignment>
                                            <IntegerVariable>waypt_id</IntegerVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>task_id</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                      <Node NodeType="Assignment"><NodeId>Get_timeout</NodeId><NodeBody>
                                          <Assignment>
                                            <RealVariable>waypt_timeout</RealVariable>
                                            <NumericRHS>
                                              <LookupNow>
                                                <Name>
<StringValue>timeout</StringValue>
                                                </Name>
                                              </LookupNow>
                                            </NumericRHS>
                                          </Assignment>
                                        </NodeBody></Node>
                                    </NodeList>
                                  </NodeBody></Node>
                                <Node NodeType="Assignment"><NodeId>Cmd_Resest1</NodeId><NodeBody>
                                    <Assignment>
                                      <IntegerVariable>cmd_return_val</IntegerVariable>
                                      <NumericRHS>
                                        <IntegerValue>-1000</IntegerValue>
                                      </NumericRHS>
                                    </Assignment>
                                  </NodeBody></Node>
                                <Node NodeType="NodeList"><NodeId>DriveAndAcquire</NodeId><StartCondition><Finished>
                                      <NodeId>GetNewWaypt</NodeId>
                                    </Finished></StartCondition><PreCondition><Succeeded>
                                      <NodeId>GetNewWaypt</NodeId>
                                    </Succeeded></PreCondition><NodeBody>
                                    <NodeList>
                                      <Node NodeType="NodeList" epx="If"><NodeId>ep2cp_If_d1e1062</NodeId><NodeBody><NodeList><Node NodeType="NodeList" epx="Then"><StartCondition><NOT><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>task_type</IntegerVariable></EQNumeric></NOT></StartCondition><SkipCondition><NOT><NOT><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>task_type</IntegerVariable></EQNumeric></NOT></NOT></SkipCondition><NodeId>ProcessWaypoint</NodeId><NodeBody>
                                              <NodeList>
                                                <Node NodeType="NodeList" epx="Sequence"><NodeId>ep2cp_Sequence_d1e1087</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="Command"><NodeId>DriveToWaypoint</NodeId><PostCondition><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></PostCondition><InvariantCondition><OR><EQNumeric><IntegerValue>-1000</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>2</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>3</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></OR></InvariantCondition><EndCondition><OR><NENumeric><IntegerValue>-1000</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></NENumeric><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>is_aborted</BooleanVariable></EQBoolean></OR></EndCondition><NodeBody>
                                                      <Command>
                                                        <IntegerVariable>cmd_return_val</IntegerVariable>
                                                        <Name>
<StringValue>move_to_waypoint</StringValue>
                                                        </Name>
                                                        <Arguments>
                                                          <StringVariable>waypt_name</StringVariable>
                                                          <StringVariable>waypt_frame</StringVariable>
                                                          <RealVariable>waypt_x</RealVariable>
                                                          <RealVariable>waypt_y</RealVariable>
                                                          <RealVariable>waypt_z</RealVariable>
                                                          <RealVariable>waypt_a</RealVariable>
                                                          <RealVariable>waypt_is_directional</RealVariable>
                                                          <RealVariable>waypt_tolerance</RealVariable>
                                                          <RealVariable>waypt_heading_tolerance</RealVariable>
                                                          <RealVariable>waypt_hinted_speed</RealVariable>
                                                        </Arguments>
                                                      </Command>
                                                    </NodeBody></Node><Node NodeType="Assignment"><NodeId>SetNameToDrive</NodeId><StartCondition><Finished><NodeRef dir="sibling">DriveToWaypoint</NodeRef></Finished></StartCondition><NodeBody>
                                                      <Assignment>
                                                        <StringVariable>cmd_name</StringVariable>
                                                        <StringRHS>
<StringValue>drive</StringValue>
                                                        </StringRHS>
                                                      </Assignment>
                                                    </NodeBody></Node><Node NodeType="NodeList"><NodeId>WrapperNode</NodeId><StartCondition><Finished><NodeRef dir="sibling">SetNameToDrive</NodeRef></Finished></StartCondition><PreCondition><Succeeded>
                                                        <NodeId>DriveToWaypoint</NodeId>
                                                      </Succeeded></PreCondition><NodeBody>
                                                      <NodeList>
                                                        <Node NodeType="NodeList" epx="If"><NodeId>ep2cp_If_d1e1229</NodeId><NodeBody><NodeList><Node NodeType="NodeList" epx="Then"><StartCondition><NOT><EQNumeric><IntegerValue>1</IntegerValue><IntegerVariable>task_type</IntegerVariable></EQNumeric></NOT></StartCondition><SkipCondition><NOT><NOT><EQNumeric><IntegerValue>1</IntegerValue><IntegerVariable>task_type</IntegerVariable></EQNumeric></NOT></NOT></SkipCondition><NodeId>ep2cp_Sequence_d1e1246</NodeId><InvariantCondition><NoChildFailed><NodeRef dir="self"/></NoChildFailed></InvariantCondition><NodeBody><NodeList><Node NodeType="Assignment"><NodeId>Cmd_Resest2</NodeId><NodeBody>
                                                                  <Assignment>
                                                                    <IntegerVariable>cmd_return_val</IntegerVariable>
                                                                    <NumericRHS>
                                                                      <IntegerValue>-1000</IntegerValue>
                                                                    </NumericRHS>
                                                                  </Assignment>
                                                                </NodeBody></Node><Node NodeType="Command"><NodeId>TakeSample</NodeId><StartCondition><Finished><NodeRef dir="sibling">Cmd_Resest2</NodeRef></Finished></StartCondition><PostCondition><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></PostCondition><InvariantCondition><OR><EQNumeric><IntegerValue>-1000</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>0</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>3</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>2</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></OR></InvariantCondition><EndCondition><OR><NENumeric><IntegerValue>-1000</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></NENumeric><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>is_aborted</BooleanVariable></EQBoolean></OR></EndCondition><NodeBody>
                                                                  <Command>
                                                                    <IntegerVariable>cmd_return_val</IntegerVariable>
                                                                    <Name>
<StringValue>acquire</StringValue>
                                                                    </Name>
                                                                  </Command>
                                                                </NodeBody></Node><Node NodeType="Assignment"><NodeId>SetNameToAcquire</NodeId><StartCondition><Finished><NodeRef dir="sibling">TakeSample</NodeRef></Finished></StartCondition><NodeBody>
                                                                  <Assignment>
                                                                    <StringVariable>cmd_name</StringVariable>
                                                                    <StringRHS>
<StringValue>acquire</StringValue>
                                                                    </StringRHS>
                                                                  </Assignment>
                                                                </NodeBody></Node></NodeList></NodeBody></Node></NodeList></NodeBody></Node>
                                                      </NodeList>
                                                    </NodeBody></Node></NodeList></NodeBody></Node>
                                              </NodeList>
                                            </NodeBody></Node></NodeList></NodeBody></Node>
                                    </NodeList>
                                  </NodeBody></Node>
                              </NodeList>
                            </NodeBody></Node>
                          <Node NodeType="Assignment"><NodeId>SetPauseResult</NodeId><StartCondition><AND><Finished>
                                  <NodeId>GetAndProcessWaypoint</NodeId>
                                </Finished><EQBoolean><BooleanValue>1</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean></AND></StartCondition><NodeBody>
                              <Assignment>
                                <IntegerVariable>cmd_return_val</IntegerVariable>
                                <NumericRHS>
                                  <IntegerValue>-3</IntegerValue>
                                </NumericRHS>
                              </Assignment>
                            </NodeBody></Node>
                          <Node NodeType="Update"><NodeId>SendUpdate</NodeId><StartCondition><OR><AND><Finished>
                                    <NodeId>GetAndProcessWaypoint</NodeId>
                                  </Finished><EQBoolean><BooleanValue>0</BooleanValue><BooleanVariable>is_paused</BooleanVariable></EQBoolean></AND><Finished>
                                  <NodeId>SetWaypointRequestFailed</NodeId>
                                </Finished><Finished>
                                  <NodeId>SetPauseResult</NodeId>
                                </Finished></OR></StartCondition><NodeBody>
                              <Update>
                                <Pair>
                                  <Name>taskId</Name>
                                  <IntegerVariable>waypt_id</IntegerVariable>
                                </Pair>
                                <Pair>
                                  <Name>result</Name>
                                  <IntegerVariable>cmd_return_val</IntegerVariable>
                                </Pair>
                                <Pair>
                                  <Name>commandName</Name>
                                  <StringVariable>cmd_name</StringVariable>
                                </Pair>
                              </Update>
                            </NodeBody></Node>
                          <Node NodeType="NodeList"><NodeId>CheckIfAbortPlan</NodeId><StartCondition><Finished>
                                <NodeId>SendUpdate</NodeId>
                              </Finished></StartCondition><NodeBody>
                              <NodeList>
                                <Node NodeType="NodeList" epx="If"><NodeId>ep2cp_If_d1e1500</NodeId><NodeBody><NodeList><Node NodeType="Assignment" epx="Then"><StartCondition><OR><EQNumeric><IntegerValue>-1</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>1</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>4</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>5</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>6</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></OR></StartCondition><SkipCondition><NOT><OR><EQNumeric><IntegerValue>-1</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>1</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>4</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>5</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric><EQNumeric><IntegerValue>6</IntegerValue><IntegerVariable>cmd_return_val</IntegerVariable></EQNumeric></OR></NOT></SkipCondition><NodeId>SetAbort</NodeId><NodeBody>
                                        <Assignment>
                                          <BooleanVariable>abort_due_to_exception</BooleanVariable>
                                          <BooleanRHS>
                                            <BooleanValue>1</BooleanValue>
                                          </BooleanRHS>
                                        </Assignment>
                                      </NodeBody></Node></NodeList></NodeBody></Node>
                              </NodeList>
                            </NodeBody></Node>
                        </NodeList>
                      </NodeBody></Node>
                  </NodeList>
                </NodeBody></Node>
              <Node NodeType="Assignment"><NodeId>MonitorPauseSignal</NodeId><StartCondition><NEBoolean><BooleanVariable>is_paused</BooleanVariable><LookupOnChange>
                      <Name>
<StringValue>plan_paused</StringValue>
                      </Name>
                    </LookupOnChange></NEBoolean></StartCondition><RepeatCondition><BooleanValue>1</BooleanValue></RepeatCondition><NodeBody>
                  <Assignment>
                    <BooleanVariable>is_paused</BooleanVariable>
                    <BooleanRHS>
                      <LookupNow>
                        <Name>
<StringValue>plan_paused</StringValue>
                        </Name>
                      </LookupNow>
                    </BooleanRHS>
                  </Assignment>
                </NodeBody></Node>
            </NodeList>
          </NodeBody></Node>
        <Node NodeType="Update"><NodeId>SignalEndOfPlan</NodeId><StartCondition><AND><Finished>
                <NodeId>SiteSurveyWrapper</NodeId>
              </Finished><Waiting>
                <NodeId>MonitorAbortSignal</NodeId>
              </Waiting></AND></StartCondition><NodeBody>
            <Update>
              <Pair>
                <Name>endOfPlan</Name>
                <BooleanValue>1</BooleanValue>
              </Pair>
            </Update>
          </NodeBody></Node>
        <Node NodeType="Assignment"><NodeId>MonitorAbortSignal</NodeId><StartCondition><EQBoolean><BooleanValue>1</BooleanValue><LookupOnChange>
                <Name>
<StringValue>abort_plan</StringValue>
                </Name>
              </LookupOnChange></EQBoolean></StartCondition><NodeBody>
            <Assignment>
              <BooleanVariable>is_aborted</BooleanVariable>
              <BooleanRHS>
                <BooleanValue>1</BooleanValue>
              </BooleanRHS>
            </Assignment>
          </NodeBody></Node>
        <Node NodeType="Update"><NodeId>SendAbortUpdate</NodeId><StartCondition><Finished>
              <NodeId>MonitorAbortSignal</NodeId>
            </Finished></StartCondition><NodeBody>
            <Update>
              <Pair>
                <Name>taskId</Name>
                <IntegerVariable>waypt_id</IntegerVariable>
              </Pair>
              <Pair>
                <Name>result</Name>
                <IntegerValue>-2</IntegerValue>
              </Pair>
            </Update>
          </NodeBody></Node>
      </NodeList>
    </NodeBody></Node></PlexilPlan>