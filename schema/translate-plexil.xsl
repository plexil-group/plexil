<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" indent="yes"/>

<!-- To Do
     incorporate into run-ue (?)

     add checks for all node states?
     add all node failure types to transition diagrams
     
     Implementation:
     copy comments (hard)
     use copy-of in local variables
-->

<xsl:key name= "action"
         match= "Node|Concurrence|Sequence|UncheckedSequence|Try|If|While|For"
         use= "."/>

<!-- Entry point -->
<xsl:template match= "PlexilPlan">
  <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
    <!-- 0 or 1 expected -->
    <xsl:copy-of select= "GlobalDeclarations"/>
    <!-- 1 expected -->
    <xsl:apply-templates select= "key('action', *)"/>
  </PlexilPlan>
</xsl:template>

<xsl:template match= "Node">
  <xsl:call-template name= "translate-node"/>
</xsl:template>

<xsl:template match= "Node" mode= "ordered">
  <xsl:call-template name= "translate-node">
    <xsl:with-param name= "ordered" select= "'true'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "translate-node">
  <xsl:param name= "ordered"/>
  <Node>
    <!-- Parts that are copied directly -->
    <xsl:copy-of select= "@NodeType"/>
    <xsl:copy-of select= "@FileName"/>
    <xsl:copy-of select= "@LineNo"/>
    <xsl:copy-of select= "@ColNo"/>
    <xsl:copy-of select= "NodeId"/>
    <xsl:copy-of select= "VariableDeclarations"/>
    <xsl:copy-of select= "Priority"/>
    <xsl:copy-of select= "Permissions"/>
    <xsl:copy-of select= "Interface"/>
    <!-- Handle start condition -->
    <xsl:choose>
      <xsl:when test= "$ordered">
        <xsl:call-template name= "ordered-start-condition"/>      
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select= "StartCondition"/>      
      </xsl:otherwise>
    </xsl:choose>
    <!-- Elements that may need translation -->
    <xsl:apply-templates select= "RepeatCondition|PreCondition|PostCondition|
                                  InvariantCondition|EndCondition|SkipCondition"/>
    <xsl:apply-templates select= "NodeBody"/>
  </Node>
</xsl:template>

<xsl:template match= "NodeBody">
  <NodeBody>
    <xsl:choose>
      <xsl:when test= "NodeList">
        <NodeList>
          <xsl:apply-templates select= "*"/>
        </NodeList>
      </xsl:when>
      <xsl:otherwise>
        <xsl:copy-of select= "*"/>
      </xsl:otherwise>
    </xsl:choose>
  </NodeBody>
</xsl:template>

<xsl:template name= "ordered-start-condition">
  <xsl:choose>
    <xsl:when test= "preceding-sibling::Node|preceding-sibling::Sequence|
                     preceding-sibling::UncheckedSequence|preceding-sibling::If|
                     preceding-sibling::While|preceding-sibling::For|
                     preceding-sibling::Try|preceding-sibling::Concurrence">
      <StartCondition>
        <AND>
          <xsl:choose>
            <xsl:when test= "preceding-sibling::*[1]/NodeId">
              <xsl:call-template name= "node-finished">
                <xsl:with-param 
                    name= "id"
                    select= "preceding-sibling::*[1]/NodeId"/>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name= "node-finished">
                <xsl:with-param 
                    name= "id"
                    select= "tr:node-id(preceding-sibling::*[1])"/>
              </xsl:call-template>
            </xsl:otherwise>
          </xsl:choose>
          <xsl:apply-templates select= "StartCondition/*"/>
        </AND>
      </StartCondition>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select= "StartCondition"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match= "UncheckedSequence">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses"/>
    <xsl:call-template name= "sequence-body"/>
  </Node>
</xsl:template>

<xsl:template match= "UncheckedSequence" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
    </xsl:call-template>
    <xsl:call-template name= "sequence-body"/>    
  </Node>
</xsl:template>

<xsl:template name= "sequence-body">
  <NodeBody>
    <NodeList>
      <xsl:apply-templates select= "key('action', *)" mode= "ordered"/>
    </NodeList>
  </NodeBody>
</xsl:template>


<xsl:template match= "Sequence">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "success-invariant" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "sequence-body"/>
  </Node>
</xsl:template>

<xsl:template match= "Sequence" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
      <xsl:with-param name= "success-invariant" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "sequence-body"/>    
  </Node>
</xsl:template>

<xsl:template match= "Concurrence">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses"/>
    <xsl:call-template name= "concurrent-body"/>
  </Node>
</xsl:template>

<xsl:template match= "Concurrence" mode="ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
    </xsl:call-template>
    <xsl:call-template name= "concurrent-body"/>
  </Node>
</xsl:template>

<xsl:template name= "concurrent-body">
  <NodeBody>
    <NodeList>
      <xsl:apply-templates select= "key('action', *)"/>
    </NodeList>
  </NodeBody>
</xsl:template>


<xsl:template match= "Try">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "try-clauses" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "sequence-body"/>
  </Node>
</xsl:template>

<xsl:template match= "Try" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
      <xsl:with-param name= "try-clauses" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "sequence-body"/>    
  </Node>
</xsl:template>


<xsl:template match= "If">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "declare-test" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "if-body"/>
  </Node>
</xsl:template>

<xsl:template match= "If" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
      <xsl:with-param name= "declare-test" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "if-body"/>    
  </Node>
</xsl:template>

<xsl:template name= "if-body">
  <xsl:variable name= "setup-node-id" select= "tr:prefix('IfSetup')"/>
  <xsl:variable name= "true-node-id" select= "tr:prefix('IfTrueCase')"/>  
  <xsl:variable name= "false-node-id" select= "tr:prefix('IfFalseCase')"/>  
  <NodeBody>
    <NodeList>
      <Node NodeType= "Assignment">
        <NodeId>
          <xsl:value-of select= "$setup-node-id"/>
        </NodeId>
        <NodeBody>
          <Assignment>
            <BooleanVariable>
              <xsl:value-of select= "tr:prefix('test')"/>
            </BooleanVariable>
            <BooleanRHS>
              <xsl:apply-templates select= "Condition/*"/>
            </BooleanRHS>
          </Assignment>
        </NodeBody>
      </Node>
      <Node NodeType= "NodeList">
        <NodeId>
          <xsl:value-of select= "tr:prefix('IfBody')"/>
        </NodeId>
        <StartCondition>
          <xsl:call-template name= "node-finished">
            <xsl:with-param name= "id" select= "$setup-node-id"/>
          </xsl:call-template>
        </StartCondition>
        <EndCondition>
          <OR>
            <xsl:call-template name= "node-finished">
              <xsl:with-param name= "id" select= "$true-node-id"/>
            </xsl:call-template>
            <xsl:choose>
              <xsl:when test= "Else">
                <xsl:call-template name= "node-finished">
                  <xsl:with-param name= "id" select= "$false-node-id"/>
                </xsl:call-template>
              </xsl:when>
              <xsl:otherwise>
                <NOT>
                  <BooleanVariable>
                    <xsl:value-of select= "tr:prefix('test')"/>
                  </BooleanVariable>
                </NOT>
              </xsl:otherwise>
            </xsl:choose>
          </OR>
        </EndCondition>
        <NodeBody>
          <NodeList>
            <Node NodeType= "NodeList">
              <NodeId>
                <xsl:value-of select= "$true-node-id"/>
              </NodeId>
              <StartCondition>
                <BooleanVariable>
                  <xsl:value-of select= "tr:prefix('test')"/>
                </BooleanVariable>
              </StartCondition>
              <NodeBody>
                <NodeList>
                  <xsl:apply-templates select= "Then/*"/>
                </NodeList>
              </NodeBody>
            </Node>
            <xsl:if test= "Else">
              <Node NodeType= "NodeList">
                <NodeId>
                  <xsl:value-of select= "$false-node-id"/>
                </NodeId>
                <StartCondition>
                  <NOT><BooleanVariable>
                    <xsl:value-of select= "tr:prefix('test')"/>
                  </BooleanVariable></NOT>
                </StartCondition>
                <NodeBody>
                  <NodeList>
                    <xsl:apply-templates select= "Else/*"/>
                  </NodeList>
                </NodeBody>
              </Node>
            </xsl:if>
          </NodeList>
        </NodeBody>
      </Node>
    </NodeList>
  </NodeBody>
</xsl:template>


<xsl:template match= "While">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses"/>
    <xsl:call-template name= "while-body"/>
  </Node>
</xsl:template>

<xsl:template match= "While" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
    </xsl:call-template>
    <xsl:call-template name= "while-body"/>    
  </Node>
</xsl:template>

<xsl:template name= "while-body">
  <xsl:variable name= "setup-node-id" select= "tr:prefix('WhileSetup')"/>
  <xsl:variable name= "true-node-id" select= "tr:prefix('WhileTrue')"/>  
  <NodeBody>
    <NodeList>
      <Node NodeType= "NodeList">
        <NodeId>
          <xsl:value-of select= "tr:prefix('WhileBody')"/>
        </NodeId>
        <NodeBody>
          <NodeList>
            <Node NodeType= "NodeList">
              <NodeId>
                <xsl:value-of select= "$true-node-id"/>
              </NodeId>
              <StartCondition>
                <xsl:apply-templates select= "Condition/*"/>                
              </StartCondition>
              <RepeatCondition>
                <xsl:apply-templates select= "Condition/*"/>                
              </RepeatCondition>
              <NodeBody>
                <NodeList>
                  <xsl:apply-templates select= "key('action', *)"/>
                </NodeList>
              </NodeBody>
            </Node>
          </NodeList>
        </NodeBody>
      </Node>
    </NodeList>
  </NodeBody>
</xsl:template>


<xsl:template match= "For">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "declare-for" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "for-body"/>
  </Node>
</xsl:template>

<xsl:template match= "For" mode= "ordered">
  <Node NodeType="NodeList">
    <xsl:call-template name= "translate-nose-clauses">
      <xsl:with-param name= "mode" select= "'ordered'"/>
      <xsl:with-param name= "declare-for" select= "'true'"/>
    </xsl:call-template>
    <xsl:call-template name= "for-body"/>    
  </Node>
</xsl:template>

<xsl:template name= "for-body">
  <xsl:variable name= "setup-node-id" select= "tr:prefix('ForSetup')"/>
  <xsl:variable name= "loop-node-id" select= "tr:prefix('ForLoop')"/>
  <xsl:variable name= "do-node-id" select= "tr:prefix('ForDo')"/>
  <NodeBody>
    <NodeList>
      <Node NodeType= "NodeList">
        <NodeId>
          <xsl:value-of select= "$loop-node-id"/>
        </NodeId>
        <RepeatCondition>
          <xsl:apply-templates select= "Condition/*"/>
        </RepeatCondition>
        <NodeBody>
          <NodeList>
            <Node NodeType= "NodeList">
              <NodeId>
                <xsl:value-of select= "$do-node-id"/>
              </NodeId>
              <NodeBody>
                <NodeList>
                  <xsl:apply-templates select= "Actions/*"/>
                </NodeList>
              </NodeBody>
            </Node>
            <Node NodeType= "Assignment">
              <NodeId>
                <xsl:value-of select= "tr:prefix('ForLoopUpdater')"/>
              </NodeId>
              <StartCondition>
                <xsl:call-template name= "node-finished">
                  <xsl:with-param name= "id" select= "$do-node-id"/>
                </xsl:call-template>
              </StartCondition>
              <NodeBody>
                <Assignment>
                  <xsl:choose>
                    <xsl:when
                        test= "LoopVariable/DeclareVariable/Type = 'Integer'">
                      <IntegerVariable>
                        <xsl:value-of
                            select= "LoopVariable/DeclareVariable/Name"/>
                      </IntegerVariable>
                    </xsl:when>
                    <xsl:when
                        test= "LoopVariable/DeclareVariable/Type = 'Real'">
                      <RealVariable>
                        <xsl:value-of
                            select= "LoopVariable/DeclareVariable/Name"/>
                      </RealVariable>
                    </xsl:when>
                    <xsl:otherwise>
                      <error>Illegal loop variable type in For</error>
                    </xsl:otherwise>
                  </xsl:choose>
                  <NumericRHS>
                    <xsl:copy-of select= "LoopVariableUpdate/*"/> 
                  </NumericRHS>
                </Assignment>
              </NodeBody>
            </Node>
          </NodeList>
        </NodeBody>
      </Node>
    </NodeList>
  </NodeBody>
</xsl:template>

<xsl:template name= "translate-nose-clauses">
  <xsl:param name= "mode"/>
  <xsl:param name= "declare-test"/>
  <xsl:param name= "declare-for"/>
  <xsl:param name= "success-invariant"/>
  <xsl:param name= "try-clauses"/>
  <!-- Copy attributes first -->
  <xsl:copy-of select= "@FileName"/>
  <xsl:copy-of select= "@LineNo"/>
  <xsl:copy-of select= "@ColNo"/>
  <!-- Then handle NodeId -->
  <xsl:call-template name= "insert-node-id"/>
  <!-- Copy clauses that don't need translation -->
  <xsl:copy-of select= "Comment"/>
  <xsl:copy-of select= "Priority"/>
  <xsl:copy-of select= "Permissions"/>
  <xsl:copy-of select= "Interface"/>
  <!-- Special case translations -->
  <xsl:choose>
    <xsl:when test= "$declare-test">
      <!-- declare a "test" variable in addition to existing -->
      <VariableDeclarations>
        <xsl:copy-of select= "VariableDeclarations/*"/>
        <xsl:call-template name= "declare-variable">
          <xsl:with-param name= "name" select= "tr:prefix('test')"/>
          <xsl:with-param name= "type" select= "'Boolean'"/>
        </xsl:call-template>
      </VariableDeclarations>
    </xsl:when>
    <xsl:when test= "$declare-for">
      <!-- declare the for loop's variable -->
      <VariableDeclarations>
        <xsl:copy-of select= "VariableDeclarations/*"/>
        <xsl:copy-of select= "LoopVariable/*"/>
      </VariableDeclarations>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select= "VariableDeclarations"/>      
    </xsl:otherwise>
  </xsl:choose>
  <!-- Handle start condition -->
  <xsl:choose>
    <xsl:when test= "$mode = 'ordered'">
      <xsl:call-template name= "ordered-start-condition"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select= "StartCondition"/>
    </xsl:otherwise>
  </xsl:choose>
  <!-- Translate remaining conditions, handling special cases -->
  <xsl:apply-templates select= "RepeatCondition"/>
  <xsl:apply-templates select= "PreCondition"/>
  <xsl:choose>
    <xsl:when test= "$try-clauses">
      <PostCondition>
        <AND>
          <xsl:apply-templates select= "PostCondition/*"/>
          <OR>
            <xsl:apply-templates
                select= "key('action', *)" mode= "success-check"/>
          </OR>
        </AND>
      </PostCondition>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select= "PostCondition"/>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
    <xsl:when test= "$success-invariant">
      <InvariantCondition>
        <AND>
          <xsl:apply-templates select= "InvariantCondition/*"/>
          <NOT>
            <OR>
              <xsl:apply-templates
                  select= "key('action', *)" mode= "failure-check"/>
            </OR>
          </NOT>
        </AND>
      </InvariantCondition>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select= "InvariantCondition"/>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
    <xsl:when test= "$try-clauses">
      <EndCondition>
        <OR>
          <xsl:apply-templates select= "EndCondition/*"/>
          <xsl:apply-templates
              select= "key('action', *)" mode= "success-check"/>
        </OR>
      </EndCondition>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select= "EndCondition"/>
    </xsl:otherwise>
  </xsl:choose>
  <xsl:apply-templates select= "SkipCondition"/>
</xsl:template>

<xsl:template match= "StartCondition|RepeatCondition|PreCondition|
                      PostCondition|InvariantCondition|EndCondition|
                      SkipCondition">
  <xsl:element name= "{name()}">
    <xsl:apply-templates select= "*"/>
  </xsl:element>
</xsl:template>

<xsl:template
    match= "Node|Concurrence|Sequence|UncheckedSequence|Try|If|While|For"
    mode= "failure-check">
  <xsl:choose>
    <xsl:when test= "NodeId">
      <xsl:call-template name= "node-failed">
        <xsl:with-param name= "id" select= "NodeId"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name= "node-failed">
        <xsl:with-param name= "id" select= "tr:node-id(.)"/>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template
    match= "Node|Concurrence|Sequence|UncheckedSequence|Try|If|While|For"
    mode= "success-check">
  <xsl:choose>
    <xsl:when test= "NodeId">
      <xsl:call-template name= "node-succeeded">
        <xsl:with-param name= "id" select= "NodeId"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name= "node-succeeded">
        <xsl:with-param name= "id" select= "tr:node-id(.)"/>
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name= "insert-node-id">
  <xsl:param name= "node" select= "."/>
  <!-- Supply missing NodeId or copy existing one -->
  <xsl:choose>
    <xsl:when test= "not(NodeId)">
      <NodeId>
        <xsl:value-of select= "tr:node-id($node)"/>
      </NodeId>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy-of select= "NodeId"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name= "declare-variable">
  <xsl:param name= "name"/>
  <xsl:param name= "type"/>
  <xsl:param name= "init-value"/>
  <DeclareVariable>
    <Name><xsl:value-of select= "$name"/></Name>
    <Type><xsl:value-of select= "$type"/></Type>
    <xsl:if test= "$init-value">
      <InitialValue>
        <xsl:element name= "{concat($type, 'Value')}">
          <xsl:value-of select= "$init-value"/>
        </xsl:element>
      </InitialValue>
    </xsl:if>
  </DeclareVariable>
</xsl:template>

<!-- Boolean Expressions -->

<!-- These expressions are translated recursively. -->
<xsl:template match= "OR|XOR|AND|NOT|EQBoolean|NEBoolean">
  <xsl:element name= "{name()}">
    <xsl:apply-templates select= "*"/>
  </xsl:element>
</xsl:template>

<!-- These expressions are deep copied. -->
<xsl:template match= "IsKnown|GT|GE|LT|LE|EQNumeric|EQInternal|EQString|
                      NENumeric|NEInternal|NEString|BooleanVariable|
                      BooleanValue|LookupNow|LookupOnChange|ArrayElement">
  <xsl:copy-of select= "."/>
</xsl:template>

<xsl:template match= "NodeFinished">
  <xsl:call-template name= "node-finished"/>
</xsl:template>


<xsl:template match= "NodeIterationEnded">
  <xsl:call-template name= "node-iteration-ended"/>
</xsl:template>

<xsl:template match= "NodeExecuting">
  <xsl:call-template name= "node-executing"/>
</xsl:template>

<xsl:template match= "NodeWaiting">
  <xsl:call-template name= "node-waiting"/>
</xsl:template>

<xsl:template match= "NodeInactive">
  <xsl:call-template name= "node-inactive"/>
</xsl:template>

<xsl:template match= "NodeSucceeded">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-succeeded"/>
  </AND>
</xsl:template>

<xsl:template match= "NodeIterationSucceeded">
  <AND>
    <xsl:call-template name= "node-iteration-ended"/>
    <xsl:call-template name= "node-succeeded"/>
  </AND>
</xsl:template>

<xsl:template match= "NodeFailed">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-failed"/>
  </AND>
</xsl:template>

<xsl:template match= "NodeIterationFailed">
  <AND>
    <xsl:call-template name= "node-iteration-ended"/>
    <xsl:call-template name= "node-failed"/>
  </AND>
</xsl:template>

<xsl:template match= "NodeSkipped">
  <!-- NOTE: implies that node is in state FINISHED. -->
  <xsl:call-template name= "node-skipped"/>
</xsl:template>

<xsl:template match= "NodeInvariantFailed">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-invariant-failed"/>
  </AND>
</xsl:template>

<xsl:template match= "NodePreconditionFailed">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-precondition-failed"/>
  </AND>
</xsl:template>

<xsl:template match= "NodePostconditionFailed">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-postcondition-failed"/>
  </AND>
</xsl:template>

<xsl:template match= "NodeParentFailed">
  <AND>
    <xsl:call-template name= "node-finished"/>
    <xsl:call-template name= "node-parent-failed"/>
  </AND>
</xsl:template>

<xsl:template name= "node-finished">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-state-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "state" select= "'FINISHED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-iteration-ended">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-state-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "state" select= "'ITERATION_ENDED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-executing">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-state-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "state" select= "'EXECUTING'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-waiting">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-state-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "state" select= "'WAITING'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-inactive">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-state-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "state" select= "'INACTIVE'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-state-check">
  <xsl:param name= "id"/>
  <xsl:param name= "state"/>
  <EQInternal>
    <NodeStateVariable>
      <NodeId><xsl:value-of select= "$id"/></NodeId>
    </NodeStateVariable>
    <NodeStateValue><xsl:value-of select= "$state"/></NodeStateValue>
  </EQInternal>
</xsl:template>

<xsl:template name= "node-failed">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-outcome-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "outcome" select= "'FAILURE'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-succeeded">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-outcome-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "outcome" select= "'SUCCESS'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-skipped">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-outcome-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "outcome" select= "'SKIPPED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-outcome-check">
  <xsl:param name= "id"/>
  <xsl:param name= "outcome"/>
  <EQInternal>
    <NodeOutcomeVariable>
      <NodeId><xsl:value-of select= "$id"/></NodeId>
    </NodeOutcomeVariable>
    <NodeOutcomeValue><xsl:value-of select= "$outcome"/></NodeOutcomeValue>
  </EQInternal>
</xsl:template>


<xsl:template name= "node-invariant-failed">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-failure-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "failure" select= "'INVARIANT_CONDITION_FAILED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-precondition-failed">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-failure-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "failure" select= "'PRE_CONDITION_FAILED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-postcondition-failed">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-failure-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "failure" select= "'POST_CONDITION_FAILED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-parent-failed">
  <xsl:param name= "id" select="text()"/>
  <xsl:call-template name= "node-failure-check">
    <xsl:with-param name= "id" select= "$id"/>
    <xsl:with-param name= "failure" select= "'PARENT_FAILED'"/>
  </xsl:call-template>
</xsl:template>

<xsl:template name= "node-failure-check">
  <xsl:param name= "id"/>
  <xsl:param name= "failure"/>
  <EQInternal>
    <NodeFailureVariable>
      <NodeId><xsl:value-of select= "$id"/></NodeId>
    </NodeFailureVariable>
    <NodeFailureValue><xsl:value-of select= "$failure"/></NodeFailureValue>
  </EQInternal>
</xsl:template>

<!-- Computes a unique NodeID -->
<xsl:function name= "tr:node-id">
  <xsl:param name= "node"/>
  <xsl:value-of select= "concat(name($node), '_', generate-id($node))"/>
</xsl:function>

<!-- Prefix names of some generated nodes and variables -->
<xsl:function name= "tr:prefix">
  <xsl:param name= "name"/>
  <xsl:value-of select= "concat('ep2cp_', $name)"/>
</xsl:function>

</xsl:transform>
