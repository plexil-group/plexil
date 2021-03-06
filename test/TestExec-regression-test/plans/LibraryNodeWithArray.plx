<PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xmlns:tr="extended-plexil-translator">
   <Node NodeType="NodeList">
      <NodeId>LibraryNodeWithArray</NodeId>
      <Interface>
         <InOut>
            <DeclareArray>
               <Name>states</Name>
               <Type>Real</Type>
               <MaxSize>4</MaxSize>
            </DeclareArray>
         </InOut>
      </Interface>
      <VariableDeclarations>
        <DeclareMutex>
          <Name>statesMutex</Name>
        </DeclareMutex>
      </VariableDeclarations>
      <NodeBody>
         <NodeList>
		   <Node NodeType="Assignment">
			 <NodeId>set_array_0</NodeId>
			 <Priority>0</Priority>
             <UsingMutex>
               <Name>statesMutex</Name>
             </UsingMutex>
			 <NodeBody>
			   <Assignment>
				 <ArrayElement>
				   <Name>states</Name>
				   <Index>
					 <IntegerValue>0</IntegerValue>
				   </Index>
				 </ArrayElement>
				 <NumericRHS>
				   <RealValue>3600.0</RealValue>
				 </NumericRHS>
			   </Assignment>
			 </NodeBody>
		   </Node>
		   <Node NodeType="Assignment">
			 <NodeId>set_array_1</NodeId>
			 <Priority>1</Priority>
             <UsingMutex>
               <Name>statesMutex</Name>
             </UsingMutex>
			 <NodeBody>
			   <Assignment>
				 <ArrayElement>
				   <Name>states</Name>
				   <Index>
					 <IntegerValue>1</IntegerValue>
				   </Index>
				 </ArrayElement>
				 <NumericRHS>
				   <RealValue>42.0</RealValue>
				 </NumericRHS>
			   </Assignment>
			 </NodeBody>
		   </Node>
		   <Node NodeType="Assignment">
			 <NodeId>set_array_2</NodeId>
			 <Priority>2</Priority>
             <UsingMutex>
               <Name>statesMutex</Name>
             </UsingMutex>
			 <NodeBody>
			   <Assignment>
				 <ArrayElement>
				   <Name>states</Name>
				   <Index>
					 <IntegerValue>2</IntegerValue>
				   </Index>
				 </ArrayElement>
				 <NumericRHS>
				   <RealValue>69.0</RealValue>
				 </NumericRHS>
			   </Assignment>
			 </NodeBody>
		   </Node>
		   <Node NodeType="Assignment">
			 <NodeId>set_array_3</NodeId>
			 <Priority>3</Priority>
             <UsingMutex>
               <Name>statesMutex</Name>
             </UsingMutex>
			 <NodeBody>
			   <Assignment>
				 <ArrayElement>
				   <Name>states</Name>
				   <Index>
					 <IntegerValue>3</IntegerValue>
				   </Index>
				 </ArrayElement>
				 <NumericRHS>
				   <RealValue>1776.0</RealValue>
				 </NumericRHS>
			   </Assignment>
			 </NodeBody>
		   </Node>
         </NodeList>
      </NodeBody>
   </Node>
</PlexilPlan>
