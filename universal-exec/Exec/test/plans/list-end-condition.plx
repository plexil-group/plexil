<?xml version="1.0" encoding="UTF-8"?>
<!-- this tests the default end condition of "all children finished" for
     list nodes-->
<PlexilPlan>
<Node NodeType="NodeList">
  <NodeId>Parent</NodeId>
  <VariableDeclarations>
      <DeclareVariable>
        <Name>x</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
  </VariableDeclarations>
  <NodeBody>
    <NodeList>
      <Node NodeType="Assignment">
	<NodeId>Child1</NodeId>
	<Priority>1</Priority>
	<NodeBody>
	  <Assignment>
	    <IntegerVariable>x</IntegerVariable>
	    <NumericRHS>
	      <ADD>
		<IntegerVariable>x</IntegerVariable>
		<IntegerValue>1</IntegerValue>
	      </ADD>
	    </NumericRHS>
	  </Assignment>
	</NodeBody>
      </Node>

      <Node NodeType="Assignment">
	<NodeId>Child2</NodeId>
	<Priority>2</Priority>
	<NodeBody>
	  <Assignment>
	    <IntegerVariable>x</IntegerVariable>
	    <NumericRHS>
	      <ADD>
		<IntegerVariable>x</IntegerVariable>
		<IntegerValue>1</IntegerValue>
	      </ADD>
	    </NumericRHS>
	  </Assignment>
	</NodeBody>
      </Node>

      <Node NodeType="Command">
        <NodeId>Test</NodeId>
        <StartCondition>
	  <AND>
	    <EQInternal>
	      <NodeStateVariable>
		<NodeId>Child1</NodeId>
	      </NodeStateVariable>
	      <NodeStateValue>FINISHED</NodeStateValue>
	    </EQInternal>
	    <EQInternal>
	      <NodeStateVariable>
		<NodeId>Child2</NodeId>
	      </NodeStateVariable>
	      <NodeStateValue>FINISHED</NodeStateValue>
	    </EQInternal>
	  </AND>
        </StartCondition>
        <NodeBody>
          <Command>
            <CommandName>test</CommandName>
	    <Arguments>
	      <IntegerVariable>x</IntegerVariable>
	    </Arguments>
	  </Command>
	</NodeBody>
      </Node>

    </NodeList>
  </NodeBody>
</Node>
</PlexilPlan>
