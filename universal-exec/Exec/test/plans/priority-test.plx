<?xml version="1.0" encoding="UTF-8"?>
<!-- this tests the default end condition of "all children finished" for
list nodes-->
<PlexilPlan>
  <Node NodeType="NodeList">
    <NodeId>Parent</NodeId>
    <VariableDeclarations>
      <DeclareVariable>
        <Name>a</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
         <DeclareVariable>
        <Name>b</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>c</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
      <DeclareVariable>
        <Name>d</Name>
        <Type>Integer</Type>
        <InitialValue>
          <IntegerValue>0</IntegerValue>
        </InitialValue>
      </DeclareVariable>
    </VariableDeclarations>
    <NodeBody>
      <NodeList>

	<!-- should execute in 1, 2, 3 order.  a should be equal to 11 -->
	<Node NodeType="NodeList">
	  <NodeId>Child1</NodeId>
	  <NodeBody>
	    <NodeList>
	      <Node NodeType="Assignment">
		<NodeId>Child1-1</NodeId>
		<Priority>1</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>a</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>a</IntegerVariable>
			<IntegerValue>2</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child1-2</NodeId>
		<Priority>2</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>a</IntegerVariable>
		    <NumericRHS>
		      <MUL>
			<IntegerVariable>a</IntegerVariable>
			<IntegerValue>3</IntegerValue>
		      </MUL>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child1-3</NodeId>
		<Priority>3</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>a</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>a</IntegerVariable>
			<IntegerValue>5</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	    </NodeList>
	  </NodeBody>
	</Node>

	<!-- should execute in 1, 3, 2 order.  a should be equal to 21 -->
	<Node NodeType="NodeList">
	  <NodeId>Child2</NodeId>
	  <NodeBody>
	    <NodeList>
	      <Node NodeType="Assignment">
		<NodeId>Child2-1</NodeId>
		<Priority>1</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>b</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>b</IntegerVariable>
			<IntegerValue>2</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child2-2</NodeId>
		<Priority>3</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>b</IntegerVariable>
		    <NumericRHS>
		      <MUL>
			<IntegerVariable>b</IntegerVariable>
			<IntegerValue>3</IntegerValue>
		      </MUL>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child2-3</NodeId>
		<Priority>2</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>b</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>b</IntegerVariable>
			<IntegerValue>5</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	    </NodeList>
	  </NodeBody>
	</Node>

	<!-- should execute in 2, 1, 3 order. a should be equal to 7-->
	<Node NodeType="NodeList">
	  <NodeId>Child3</NodeId>
	  <NodeBody>
	    <NodeList>
	      <Node NodeType="Assignment">
		<NodeId>Child3-1</NodeId>
		<Priority>2</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>c</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>c</IntegerVariable>
			<IntegerValue>2</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child3-2</NodeId>
		<Priority>1</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>c</IntegerVariable>
		    <NumericRHS>
		      <MUL>
			<IntegerVariable>c</IntegerVariable>
			<IntegerValue>3</IntegerValue>
		      </MUL>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child3-3</NodeId>
		<Priority>3</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>c</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>c</IntegerVariable>
			<IntegerValue>5</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	    </NodeList>
	  </NodeBody>
	</Node>

	<!-- should execute in 3, 2, 1 order.  a should be equal to 17-->
	<Node NodeType="NodeList">
	  <NodeId>Child4</NodeId>
	  <NodeBody>
	    <NodeList>
	      <Node NodeType="Assignment">
		<NodeId>Child4-1</NodeId>
		<Priority>3</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>d</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>d</IntegerVariable>
			<IntegerValue>2</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child4-2</NodeId>
		<Priority>2</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>d</IntegerVariable>
		    <NumericRHS>
		      <MUL>
			<IntegerVariable>d</IntegerVariable>
			<IntegerValue>3</IntegerValue>
		      </MUL>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	      <Node NodeType="Assignment">
		<NodeId>Child4-3</NodeId>
		<Priority>1</Priority>
		<NodeBody>
		  <Assignment>
		    <IntegerVariable>d</IntegerVariable>
		    <NumericRHS>
		      <ADD>
			<IntegerVariable>d</IntegerVariable>
			<IntegerValue>5</IntegerValue>
		      </ADD>
		    </NumericRHS>
		  </Assignment>
		</NodeBody>
	      </Node>

	    </NodeList>
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
	      <EQInternal>
		<NodeStateVariable>
		  <NodeId>Child3</NodeId>
		</NodeStateVariable>
		<NodeStateValue>FINISHED</NodeStateValue>
	      </EQInternal>
	      <EQInternal>
		<NodeStateVariable>
		  <NodeId>Child4</NodeId>
		</NodeStateVariable>
		<NodeStateValue>FINISHED</NodeStateValue>
	      </EQInternal>
	    </AND>
	  </StartCondition>
	  <NodeBody>
	    <Command>
	      <CommandName>test</CommandName>
	      <Arguments>
		<IntegerVariable>a</IntegerVariable>
		<IntegerVariable>b</IntegerVariable>
		<IntegerVariable>c</IntegerVariable>
		<IntegerVariable>d</IntegerVariable>
	      </Arguments>
	    </Command>
	  </NodeBody>
	</Node>

      </NodeList>
    </NodeBody>
  </Node>
</PlexilPlan>
