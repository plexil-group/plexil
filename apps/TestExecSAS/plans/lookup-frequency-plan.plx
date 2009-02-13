<?xml version="1.0" encoding="UTF-8"?>
<PlexilPlan>
  <Node NodeType="Command">
    <NodeId>FrequencyLookupNode</NodeId>
    <InvariantCondition>
      <LE>
	<LookupWithFrequency>
	  <StateName>temperature</StateName>
	  <Frequency>
	    <Low>
	      <RealValue>5</RealValue>
	    </Low>
	    <High>
	      <RealValue>1</RealValue>
	    </High>
	  </Frequency>
	</LookupWithFrequency>
	<RealValue>100</RealValue>
      </LE>
    </InvariantCondition>
    <NodeBody>
      <Command>
	<CommandName>sit_and_wait</CommandName>
      </Command>
    </NodeBody>
  </Node>
</PlexilPlan>
