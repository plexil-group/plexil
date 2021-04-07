<?xml version="1.0" encoding="ISO-8859-1"?>

<!--
* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-->

<!-- This stylesheet requires XSLT 2.0 for xsl:function, xsl:sequence -->
<!-- This stylesheet requires XPath 2.0 for 'except' -->

<!-- TODO:
     Ensure attributes are sorted in output; simplifies verification.
     Only outermost Node elements need a gensym'd NodeId; others can use constant ids.
-->

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

  <!-- Let the application running the translator determine whether to
       indent or not. -->
  <xsl:output method="xml"/>

  <!-- This is the "overriding copy idiom", from "XSLT Cookbook" by
       Sal Mangano.  It is the identity transform, covering all
       elements that are not explicitly handled elsewhere. -->
  <xsl:template match="node() | @*">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <!-- Entry point -->

  <xsl:template match="PlexilPlan">
    <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
      <xsl:copy-of select="@FileName" />
      <!-- 0 or 1 expected -->
      <xsl:copy-of select="GlobalDeclarations"/>
      <!-- 1 expected -->
      <xsl:apply-templates select="tr:actions(.)"/>
    </PlexilPlan>
  </xsl:template>

  <!-- ********************* -->
  <!-- High level constructs -->
  <!-- ********************* -->

  <!-- Concurrence -->
  <!-- Simply expands to a NodeList -->
  <xsl:template match="Concurrence">
    <Node NodeType="NodeList" epx="Concurrence">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="standard-preamble" />
      <NodeBody>
        <NodeList>
          <xsl:apply-templates select="tr:actions(.)" />
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- Sequences in their own file -->
  <xsl:include href="sequences.xsl" />

  <!-- Try -->
  <xsl:include href="try.xsl" />

  <!-- If-Then-Elseif-Else -->
  <xsl:include href="if-then-else.xsl" />

  <!-- ****************** -->
  <!-- Looping constructs -->
  <!-- ****************** -->

  <!-- While -->
  <xsl:include href="while.xsl" />

  <!-- Do-while -->
  <xsl:include href="do.xsl" />

  <!-- For -->
  <xsl:include href="for.xsl" />

  <!-- Wait -->
  <xsl:include href="wait.xsl" />

  <!-- SynchronousCommand -->
  <xsl:include href="synchronous-command.xsl" />

  <!--
    Support for message passing between executives
  -->

  <!-- OnMessage -->
  <xsl:include href="on-message.xsl" />

  <!-- OnCommand -->
  <xsl:include href="on-command.xsl" />

  <!-- ******************** -->
  <!-- Extended expressions -->
  <!-- ******************** -->

  <!-- Dates and Durations -->
  <xsl:include href="date-duration.xsl" />

  <!-- Predicates on node state, outcome, failure -->
  <xsl:include href="node-predicates.xsl" />

  <!-- Generic Lookup form -->
  <xsl:template match="Lookup">
    <xsl:choose>
      <!-- These are contexts in which an immediate, one-time lookup is required -->
      <!-- Not sure about InvariantCondition -->
      <xsl:when test="ancestor::Command|ancestor::Assignment|ancestor::Update|
                      ancestor::PreCondition|ancestor::Condition|
                      ancestor::PostCondition|ancestor::InvariantCondition">
        <LookupNow epx="Lookup">
          <Name>
            <xsl:apply-templates select="Name/*"/>
          </Name>
          <xsl:if test="Arguments">
            <Arguments>
              <xsl:apply-templates select="Arguments/*"/>
            </Arguments>
          </xsl:if>
        </LookupNow>
      </xsl:when>
      <xsl:otherwise>
        <LookupOnChange epx="Lookup">
          <Name>
            <xsl:apply-templates select="Name/*"/>
          </Name>
          <xsl:apply-templates select="Tolerance"/>
          <xsl:if test="Arguments">
            <Arguments>
              <xsl:apply-templates select="Arguments/*"/>
            </Arguments>
          </xsl:if>
        </LookupOnChange>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Warning:  This one might be obsolete. -->
  <xsl:template match="MessageReceived">
    <LookupOnChange>
      <Name>
        <Concat>
          <StringValue>MESSAGE__</StringValue>
          <xsl:sequence select="*" />
        </Concat>
      </Name>
    </LookupOnChange>
  </xsl:template>

  <!-- ************** -->
  <!-- Implementation -->
  <!-- ************** -->

  <!-- Special handling for certain elements -->

  <!-- apply-templates is often used *after* some other expansion,
       so ensure the attributes added by templates aren't removed -->
  <xsl:template match="Node">
    <Node>
      <!-- Copy attributes in sorted order to simplify regression test -->
      <xsl:perform-sort select="@*">
        <xsl:sort select="name(.)" />
      </xsl:perform-sort>
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="standard-preamble" />
      <xsl:apply-templates select="NodeBody" />
    </Node>
  </xsl:template>
  
  <!-- It appears these templates are redundant with the overriding copy idiom. -->

  <!-- Not clear why this is needed. -->
  <!-- <xsl:template match="Interface"> -->
  <!--   <Interface> -->
  <!--     <xsl:comment>Processed by redundant Interface template</xsl:comment> -->
  <!--     <xsl:apply-templates select="In"/>	 -->
  <!--     <xsl:apply-templates select="InOut"/> -->
  <!--   </Interface> -->
  <!-- </xsl:template> -->

  <!-- Not clear why this is needed. -->
  <!-- <xsl:template match="In"> -->
  <!--   <In> -->
  <!--     <xsl:comment>Processed by redundant In template</xsl:comment> -->
  <!--     <xsl:apply-templates select="*"/> -->
  <!--   </In> -->
  <!-- </xsl:template> -->

  <!-- Not clear why this is needed. -->
  <!-- <xsl:template match="InOut"> -->
  <!--   <InOut> -->
  <!--     <xsl:comment>Processed by redundant InOut template</xsl:comment> -->
  <!--     <xsl:apply-templates select="*"/> -->
  <!--   </InOut> -->
  <!-- </xsl:template> -->
  
  <!-- Not clear why this is needed. -->
  <!-- <xsl:template -->
  <!--     match="EndCondition|ExitCondition|InvariantCondition| -->
  <!--            PostCondition|PreCondition|RepeatCondition| -->
  <!--            SkipCondition|StartCondition"> -->
  <!--   <xsl:element name="{name()}"> -->
  <!--     <xsl:copy select="@*"/> -->
  <!--     <xsl:apply-templates select="*" /> -->
  <!--   </xsl:element> -->
  <!-- </xsl:template> -->

  <!-- Boolean Expressions -->

  <!-- These expressions are translated recursively. -->
  <!-- Why is this not handled by the overriding copy idiom?? -->
  <!-- <xsl:template match="OR|XOR|AND|NOT| -->
  <!--                      EQNumeric|EQInternal|EQString|EQBoolean|EQArray| -->
  <!--                      NENumeric|NEInternal|NEString|NEBoolean|NEArray| -->
  <!--                      GT|GE|LT|LE| -->
  <!--                      IsKnown|ALL_KNOWN|ANY_KNOWN"> -->
  <!--   <xsl:element name="{name()}"> -->
  <!--     <xsl:apply-templates select="*" /> -->
  <!--   </xsl:element> -->
  <!-- </xsl:template> -->

  <!-- These expressions are deep copied. (But must also be processed
       for dates and durations) -->
  <!-- Why is this not handled by the overriding copy idiom?? -->
  <!-- <xsl:template match="BooleanVariable|BooleanValue|LookupOnChange|LookupNow|ArrayElement"> -->
  <!--   <xsl:copy> -->
  <!--     <xsl:apply-templates /> -->
  <!--   </xsl:copy> -->
  <!-- </xsl:template> -->

  <!-- ***************** -->
  <!-- Utility templates -->
  <!-- ***************** -->

  <!--
      Recursive template that attempts to find and insert the NodeId of
      the parent action to the given node
  -->
  <xsl:template name="parent-id-value">
    <xsl:param name="start_path" select="." />
    <xsl:choose>
      <!-- Insert NodeId via insert-node-id template -->
      <xsl:when test="tr:actions($start_path/..)">
        <xsl:variable name="id">
          <xsl:call-template name="node-id">
            <xsl:with-param name="context" select="$start_path/.." />
          </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="$id/NodeId" />
      </xsl:when>
      <!-- If not an action, go up a level and try again, if one exists -->
      <xsl:when test="$start_path/..">
        <xsl:call-template name="parent-id-value">
          <xsl:with-param name="start_path" select="$start_path/.." />
        </xsl:call-template>
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- Used by Wait and SynchronousCommand -->

  <xsl:template name="timed-out">
    <xsl:param name="element"/>
    <GE>
      <LookupOnChange>
        <Name>
          <StringValue>time</StringValue>
        </Name>
        <xsl:choose>
          <xsl:when test="Tolerance">
            <xsl:apply-templates select="Tolerance"/>
          </xsl:when>
          <xsl:otherwise>
            <Tolerance>
	          <xsl:apply-templates select="$element"/>
            </Tolerance>
          </xsl:otherwise>
        </xsl:choose>
      </LookupOnChange>
      <ADD>
        <xsl:apply-templates select="$element"/>
        <NodeTimepointValue>
          <NodeRef dir="self"/>
          <NodeStateValue>EXECUTING</NodeStateValue>
          <Timepoint>START</Timepoint>
        </NodeTimepointValue>
      </ADD>
    </GE>
  </xsl:template>

  <!-- Processing for the standard parts of an action -->

  <!-- Includes variable declarations, UsingMutex, and conditions -->
  <xsl:template name="standard-preamble">
    <xsl:param name="context" select="." />
    <xsl:call-template name="handle-common-clauses">
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
    <xsl:apply-templates select="$context/(VariableDeclarations|UsingMutex)" />
    <xsl:apply-templates select="tr:conditions($context)" />
  </xsl:template>

  <xsl:template name="copy-source-locator-attributes">
    <xsl:param name="context" select="." />
    <xsl:copy-of select="$context/(@FileName|@LineNo|@ColNo)" />
  </xsl:template>

  <xsl:template name="default-node-id">
    <xsl:param name="context" select="." />
    <xsl:param name="name" required="yes" />
    <xsl:choose>
      <xsl:when test="$context/NodeId">
        <xsl:sequence select="$context/NodeId" />
      </xsl:when>
      <xsl:otherwise>
        <NodeId generated="1"><xsl:value-of select="tr:prefix($name)" /></NodeId>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Copies Comment, Priority, Assume, Desire, Expect verbatim;
       applies templates to Interface -->
  <xsl:template name="handle-common-clauses">
    <xsl:param name="context" select="." />
    <xsl:sequence select="$context/(Comment|Priority|Assume|Desire|Expect)" />
    <xsl:apply-templates select="$context/Interface"/>
  </xsl:template>

  <!-- This template should NEVER generate a new node ID. -->
  <xsl:template name="node-id">
    <xsl:param name="context" select="." />
    <xsl:value-of select="$context/NodeId" />
  </xsl:template>

  <!-- Should be invoked at top level of template output. -->
  <xsl:template name="ensure-unique-node-id">
    <xsl:param name="node" select="." />
    <!-- Supply missing NodeId or use existing one -->
    <xsl:choose>
      <!-- No NodeId element, gensym a node id -->
      <xsl:when test="not($node/NodeId)">
        <NodeId generated="1">
          <xsl:value-of select="tr:gen-node-id($node)" />
        </NodeId>
      </xsl:when>
      <!-- use existing -->
      <xsl:otherwise>
        <xsl:sequence select="$node/NodeId" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="declare-variable">
    <xsl:param name="name" />
    <xsl:param name="type" />
    <xsl:param name="init-value" />
    <DeclareVariable>
      <Name>
        <xsl:value-of select="$name" />
      </Name>
      <Type>
        <xsl:value-of select="$type" />
      </Type>
      <xsl:if test="$init-value">
        <InitialValue>
          <xsl:element name="{concat($type, 'Value')}">
            <xsl:value-of select="$init-value" />
          </xsl:element>
        </InitialValue>
      </xsl:if>
    </DeclareVariable>
  </xsl:template>

  <!-- ********* -->
  <!-- Functions -->
  <!-- ********* -->

  <!-- Function to select actions -->
  <xsl:function name="tr:actions" as="element()*">
    <xsl:param name="context" />
    <xsl:sequence
        select="$context/(Node|Concurrence|Sequence|CheckedSequence|UncheckedSequence|
                Try|If|Do|While|For|OnCommand|OnMessage|
                Wait|SynchronousCommand)" />
  </xsl:function>

  <!-- Function to select user conditions -->
  <xsl:function name="tr:conditions" as="element()*">
    <xsl:param name="context" />
    <xsl:sequence
        select="$context/(EndCondition|ExitCondition|InvariantCondition|
                PostCondition|PreCondition|RepeatCondition|
                SkipCondition|StartCondition)" />
  </xsl:function>

  <!-- Computes a unique NodeID -->
  <xsl:function name="tr:gen-node-id">
    <xsl:param name="node" />
    <xsl:value-of
      select="tr:prefix(concat(name($node), '_', generate-id($node)))" />
  </xsl:function>

  <!-- Prefix names of some generated nodes and variables -->
  <xsl:function name="tr:prefix">
    <xsl:param name="name" />
    <xsl:value-of select="concat('ep2cp_', $name)" />
  </xsl:function>

</xsl:transform>
