<?xml version="1.0" encoding="ISO-8859-1"?>

<!--
* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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
<!-- This stylesheet requires XPath 2.0 for 'intersect' operator -->

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               exclude-result-prefixes="xs">

  <xsl:output method="xml" indent="no"/>
  <!-- <xsl:output method="xml" indent="yes"/> FOR TESTING ONLY -->

  <!-- This is the "overriding copy idiom", from "XSLT Cookbook" by
       Sal Mangano.  It is the identity transform, covering all
       elements that are not explicitly handled elsewhere. -->

  <xsl:template match="node() | @*">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <!-- Abstraction for Action constructs.  Unfortunately, this is not
       allowed in 'select' attributes, so the actions are enumerated
       there. -->

  <xsl:key name="action"
           match="Node|Concurrence|
                  Sequence|CheckedSequence|UncheckedSequence|
                  Try|If|While|For|OnCommand|OnMessage|
                  Wait|SynchronousCommand"
           use="." />

  <!-- Entry point -->
  <xsl:template match="PlexilPlan">
    <PlexilPlan xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
      <xsl:copy-of select="@FileName" />
      <!-- 0 or 1 expected -->
      <xsl:copy-of select="GlobalDeclarations"/>
      <!-- 1 expected -->
      <xsl:apply-templates select="key('action', *)">
        <xsl:with-param name="mode" select="'unordered'" />
      </xsl:apply-templates>
    </PlexilPlan>
  </xsl:template>

  <xsl:template match="Node">
    <xsl:param name="mode" select="'unordered'" />
    <xsl:call-template name="translate-node">
      <xsl:with-param name="mode" select="$mode" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="translate-node">
    <xsl:param name="mode" />
    <Node>
      <!-- Parts that are copied directly -->
      <xsl:copy-of select="@NodeType" />
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations"/>
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:apply-templates select="NodeBody" />
    </Node>
  </xsl:template>

  <!-- Sequence, CheckedSequence, UncheckedSequence -->

  <xsl:template match="UncheckedSequence">
    <xsl:param name="mode" />
    <xsl:call-template name="UncheckedSequence">
      <xsl:with-param name="mode" select="$mode" />
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template name="UncheckedSequence">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template match="Sequence|CheckedSequence">
    <xsl:param name="mode" />
    <xsl:call-template name="CheckedSequence">
      <xsl:with-param name="mode" select="$mode" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="CheckedSequence">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:choose>
        <xsl:when test="$mode='ordered'">
          <xsl:call-template name="ordered-start-condition" />
          <xsl:call-template name="ordered-skip-condition" />
          <xsl:apply-templates select="RepeatCondition|PreCondition|PostCondition|
                                       ExitCondition|EndCondition" />
        </xsl:when>
        <xsl:when test="$mode='unordered'">
          <xsl:apply-templates select="StartCondition|SkipCondition|
                                       RepeatCondition|PreCondition|PostCondition|
                                       ExitCondition|EndCondition" />
        </xsl:when>
      </xsl:choose>
      <xsl:call-template name="success-invariant-condition" />
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template name="success-invariant-condition">
    <InvariantCondition>
      <xsl:choose>
        <xsl:when test="InvariantCondition/*">
          <AND>
            <xsl:apply-templates select="InvariantCondition/*" />
            <NoChildFailed> <NodeRef dir="self" /> </NoChildFailed>
          </AND>
        </xsl:when>
        <xsl:otherwise>
          <NoChildFailed> <NodeRef dir="self" /> </NoChildFailed> 
        </xsl:otherwise>
      </xsl:choose>
    </InvariantCondition>
  </xsl:template>

  <xsl:template match="Concurrence">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="Concurrence">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:call-template name="concurrent-body" />
    </Node>
  </xsl:template>

  <xsl:template name="concurrent-body">
    <NodeBody>
      <NodeList>
        <xsl:for-each select="child::* intersect key('action', *)">
          <xsl:apply-templates select=".">
            <xsl:with-param name="mode" select="'unordered'" />
          </xsl:apply-templates>
        </xsl:for-each>
      </NodeList>
    </NodeBody>
  </xsl:template>


  <xsl:template match="Try">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="Try">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:choose>
        <xsl:when test="$mode='ordered'">
          <xsl:call-template name="ordered-start-condition" />
          <xsl:call-template name="ordered-skip-condition" />
        </xsl:when>
        <xsl:when test="$mode='unordered'">
          <xsl:apply-templates
              select="StartCondition|SkipCondition" />
        </xsl:when>
      </xsl:choose>
      <xsl:apply-templates
          select="RepeatCondition|PreCondition|
                  ExitCondition|InvariantCondition" />
      <xsl:call-template name="try-end-condition" />
      <xsl:call-template name="try-post-condition" />
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template name="try-end-condition">
    <xsl:variable name="children"
                  select="child::* intersect key('action', *)" />
    <xsl:choose>
      <xsl:when test="count($children) = 1">
        <!-- No test needed, default all-children-finished is
             sufficient -->
        <xsl:apply-templates select="EndCondition" />
      </xsl:when>
      <xsl:otherwise>
        <EndCondition>
          <OR>
            <xsl:apply-templates select="EndCondition/*" />
            <xsl:for-each select="$children">
              <xsl:variable name="kid-ref">
                <NodeRef dir="child">
                  <xsl:call-template name="node-id" />
                </NodeRef>
              </xsl:variable>
              <!-- sufficient to check last child has finished -->
              <xsl:element name="{if (following-sibling::*) then 'Succeeded' else 'Finished'}">
                <xsl:sequence select="$kid-ref" />
              </xsl:element>
            </xsl:for-each>
          </OR>
        </EndCondition>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="try-post-condition">
    <PostCondition>
      <xsl:choose>
        <xsl:when test="PostCondition">
          <AND>
            <xsl:apply-templates select="PostCondition/*" />
            <xsl:call-template name="try-post-test" />
          </AND>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="try-post-test" />
        </xsl:otherwise>
      </xsl:choose>
    </PostCondition>
  </xsl:template>
  
  <xsl:template name="try-post-test">
    <xsl:variable name="post-tests">
      <xsl:for-each select="child::* intersect key('action', *)">
        <Succeeded>
          <NodeRef dir="child">
            <xsl:call-template name="node-id" />
          </NodeRef>
        </Succeeded>
      </xsl:for-each>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="count($post-tests/*) = 1">
        <xsl:sequence select="$post-tests" />
      </xsl:when>
      <xsl:otherwise>
        <OR>
          <xsl:sequence select="$post-tests" />
        </OR>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template name="sequence-body">
    <NodeBody>
      <NodeList>
        <xsl:for-each select="child::* intersect key('action', *)">
          <xsl:apply-templates select=".">
            <xsl:with-param name="mode" select="'ordered'" />
          </xsl:apply-templates>
        </xsl:for-each>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template match="If">
    <xsl:param name="mode" />
    <xsl:choose>
      <!-- General case -->
      <xsl:when test="ElseIf">
        <xsl:call-template name="generalized-if">
          <xsl:with-param name="mode" select="$mode" />
        </xsl:call-template>
      </xsl:when>

      <!-- If-Then-Else -->
      <!-- Requires a wrapper node, but can skip the test node. -->
      <xsl:when test="Else">
        <xsl:call-template name="basic-if-then-else">
          <xsl:with-param name="mode" select="$mode" />
        </xsl:call-template>
      </xsl:when>

      <!-- If-Then -->
      <xsl:otherwise>
        <xsl:call-template name="basic-if-then">
          <xsl:with-param name="mode" select="$mode" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="basic-if-then">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <NodeBody>
        <NodeList>
          <xsl:call-template name="basic-if-then-body" />
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="basic-if-then-body">
    <xsl:variable name="condition">
      <xsl:apply-templates select="Condition/*" />
    </xsl:variable>
    <!-- TODO: default missing top level node names to 'ep2cp-Then-'... -->
    <xsl:variable name="expanded-then">
      <xsl:apply-templates select="Then/*" />
    </xsl:variable>
    <xsl:for-each select="$expanded-then/*">
      <Node NodeType="{@NodeType}" epx="Then">
        <xsl:call-template name="copy-source-locator-attributes">
          <xsl:with-param name="context" select="." />
        </xsl:call-template>
        <xsl:call-template name="if-then-start-condition">
          <xsl:with-param name="condition" select="$condition" />
        </xsl:call-template>
        <xsl:call-template name="if-then-skip-condition">
          <xsl:with-param name="condition" select="$condition" />
        </xsl:call-template>
        <xsl:sequence select="* except StartCondition|SkipCondition" />
      </Node>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="if-then-start-condition">
    <xsl:param name="condition" />
    <StartCondition>
      <xsl:choose>
        <xsl:when test="StartCondition">
          <AND>
            <xsl:sequence select="$condition" />
            <xsl:apply-templates select="StartCondition/*" />
          </AND>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="$condition" />
        </xsl:otherwise>
      </xsl:choose>
    </StartCondition>
  </xsl:template>

  <xsl:template name="if-then-skip-condition">
    <xsl:param name="condition" />
    <xsl:variable name="not-condition">
      <NOT>
        <xsl:sequence select="$condition" />
      </NOT>
    </xsl:variable>
    <SkipCondition>
      <xsl:choose>
        <xsl:when test="SkipCondition">
          <OR>
            <xsl:sequence select="$not-condition" />
            <xsl:sequence select="SkipCondition/*" />
          </OR>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="$not-condition" />
        </xsl:otherwise>
      </xsl:choose>
    </SkipCondition>
  </xsl:template>
  
  <xsl:template name="basic-if-then-else">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <NodeBody>
        <NodeList>
          <xsl:call-template name="basic-if-then-body" />
          <xsl:call-template name="basic-if-else-body" />
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="basic-if-else-body">
    <xsl:variable name="condition">
      <xsl:apply-templates select="Condition/*" />
    </xsl:variable>
    <!-- TODO: default missing top level node names to 'ep2cp-Else-'... -->
    <xsl:variable name="expanded-else">
      <xsl:apply-templates select="Else/*" />
    </xsl:variable>
    <xsl:for-each select="$expanded-else/*">
      <Node NodeType="{@NodeType}" epx="Else">
        <xsl:call-template name="copy-source-locator-attributes">
          <xsl:with-param name="context" select="." />
        </xsl:call-template>
        <xsl:call-template name="if-else-start-condition">
          <xsl:with-param name="condition" select="$condition" />
        </xsl:call-template>
        <xsl:call-template name="if-else-skip-condition">
          <xsl:with-param name="condition" select="$condition" />
        </xsl:call-template>
        <xsl:sequence select="* except StartCondition|SkipCondition" />
      </Node>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="if-else-start-condition">
    <xsl:param name="condition" />
    <xsl:variable name="not-condition">
      <NOT>
        <xsl:sequence select="$condition" />
      </NOT>
    </xsl:variable>
    <StartCondition>
      <xsl:choose>
        <xsl:when test="StartCondition">
          <AND>
            <xsl:sequence select="$not-condition" />
            <xsl:apply-templates select="StartCondition/*" />
          </AND>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="$not-condition" />
        </xsl:otherwise>
      </xsl:choose>
    </StartCondition>
  </xsl:template>

  <xsl:template name="if-else-skip-condition">
    <xsl:param name="condition" />
    <SkipCondition>
      <xsl:choose>
        <xsl:when test="SkipCondition">
          <OR>
            <xsl:sequence select="$condition" />
            <xsl:sequence select="SkipCondition/*" />
          </OR>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="$condition" />
        </xsl:otherwise>
      </xsl:choose>
    </SkipCondition>
  </xsl:template>

  <xsl:template name="generalized-if">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:call-template name="if-body" />
    </Node>
  </xsl:template>

  <xsl:template name="if-body">
    <xsl:variable name="test-node-id">
      <xsl:value-of select="tr:prefix('IfTest')" />
    </xsl:variable>
    <xsl:variable name="test-node-ref">
      <NodeRef dir="sibling">
        <xsl:value-of select="$test-node-id" />
      </NodeRef>
    </xsl:variable>
    <xsl:variable name="test-true-cond">
      <Succeeded>
        <xsl:sequence select="$test-node-ref" />
      </Succeeded>
    </xsl:variable>
    <xsl:variable name="test-false-cond">
      <PostconditionFailed>
        <xsl:sequence select="$test-node-ref" />
      </PostconditionFailed>
    </xsl:variable>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId>
            <xsl:value-of select="$test-node-id" />
          </NodeId>
          <PostCondition>
            <xsl:apply-templates select="Condition/*" />
          </PostCondition>
        </Node>
        <xsl:for-each select="Then">
          <xsl:call-template name="if-clause-body">
            <xsl:with-param name="start-condition" select="$test-true-cond"/>
            <xsl:with-param name="skip-condition" select="$test-false-cond"/>
          </xsl:call-template>
        </xsl:for-each>
        <xsl:for-each select="ElseIf">
          <xsl:call-template name="elseif-clause" />
        </xsl:for-each>
        <xsl:for-each select="Else">
          <xsl:call-template name="else-clause" />
        </xsl:for-each>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template name="elseif-test-node-id">
    <xsl:param name="element" select="." />
    <xsl:value-of select="tr:prefix('ElseIf-')" />
    <xsl:value-of
        select="count($element/preceding-sibling::ElseIf)
                + 1" />
  </xsl:template>

  <xsl:template name="elseif-clause">
    <xsl:variable name="test-node-id">
      <xsl:call-template name="elseif-test-node-id" />
    </xsl:variable>
    <xsl:variable name="preceding-test-node-id">
      <xsl:choose>
        <xsl:when test="preceding-sibling::ElseIf">
          <xsl:call-template name="elseif-test-node-id">
            <xsl:with-param name="element"
                            select="preceding-sibling::ElseIf[1]" />
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="tr:prefix('IfTest')"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="preceding-test-ref">
      <NodeRef dir="sibling">
        <xsl:value-of select="$preceding-test-node-id" />
      </NodeRef>
    </xsl:variable>
    <xsl:variable name="test-node-ref">
      <NodeRef dir="sibling">
        <xsl:value-of select="$test-node-id" />
      </NodeRef>
    </xsl:variable>
    <xsl:variable name="test-node-succeeded">
      <Succeeded>
        <xsl:sequence select="$test-node-ref" />
      </Succeeded>
    </xsl:variable>
    <Node NodeType="Empty" epx="ElseIf">
      <NodeId>
        <xsl:value-of select="$test-node-id" />
      </NodeId>
      <StartCondition>
        <PostconditionFailed>
          <xsl:sequence select="$preceding-test-ref" />
        </PostconditionFailed>
      </StartCondition>
      <SkipCondition>
        <xsl:choose>
          <xsl:when test="preceding-sibling::ElseIf">
            <OR>
              <Skipped>
                <xsl:sequence select="$preceding-test-ref" />
              </Skipped>
              <Succeeded>
                <xsl:sequence select="$preceding-test-ref" />
              </Succeeded>
            </OR>
          </xsl:when>
          <xsl:otherwise>
            <Succeeded>
              <xsl:sequence select="$preceding-test-ref" />
            </Succeeded>
          </xsl:otherwise>
        </xsl:choose>
      </SkipCondition>
      <PostCondition>
        <xsl:apply-templates select="Condition/*" />
      </PostCondition>
    </Node>
    <xsl:for-each select="Then">
      <xsl:call-template name="if-clause-body">
        <xsl:with-param name="start-condition"
                        select="$test-node-succeeded" />
        <xsl:with-param name="skip-condition">
          <OR>
            <Skipped>
              <xsl:sequence select="$test-node-ref" />
            </Skipped>
            <PostconditionFailed>
              <xsl:sequence select="$test-node-ref" />
            </PostconditionFailed>
          </OR>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="else-clause">
    <xsl:choose>
      <xsl:when test="preceding-sibling::ElseIf">
        <xsl:variable name="preceding-test-ref">
          <NodeRef dir="sibling">
            <xsl:call-template name="elseif-test-node-id" >
              <xsl:with-param name="element"
                              select="preceding-sibling::ElseIf[1]" />
            </xsl:call-template>
          </NodeRef>
        </xsl:variable>
        <xsl:call-template name="if-clause-body">
          <xsl:with-param name="start-condition">
            <PostconditionFailed>
              <xsl:sequence select="$preceding-test-ref" />
            </PostconditionFailed>
          </xsl:with-param>
          <xsl:with-param name="skip-condition">
            <OR>
              <Skipped>
                <xsl:sequence select="$preceding-test-ref" />
              </Skipped>
              <Succeeded>
                <xsl:sequence select="$preceding-test-ref" />
              </Succeeded>
            </OR>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="if-test-node-ref">
          <NodeRef dir="sibling">
            <xsl:value-of select="tr:prefix('IfTest')" />
          </NodeRef>
        </xsl:variable>
        <xsl:call-template name="if-clause-body">
          <xsl:with-param name="start-condition">
            <PostconditionFailed>
              <xsl:sequence select="$if-test-node-ref" />
            </PostconditionFailed>
          </xsl:with-param>
          <xsl:with-param name="skip-condition">
            <Succeeded>
              <xsl:sequence select="$if-test-node-ref" />
            </Succeeded>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="if-clause-body">
    <xsl:param name="start-condition" required="yes" />
    <xsl:param name="skip-condition" required="yes" />
    <xsl:variable name="expanded-clause">
      <xsl:apply-templates />
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$expanded-clause/Node/(StartCondition|SkipCondition)">
        <!-- must create wrapper node -->
        <Node NodeType="NodeList" epx="{name(.)}">
          <NodeId><xsl:value-of select="tr:prefix(name(.))" /></NodeId>
          <StartCondition>
            <xsl:sequence select="$start-condition" />
          </StartCondition>
          <SkipCondition>
            <xsl:sequence select="$skip-condition" />
          </SkipCondition>
          <NodeBody>
            <NodeList>
              <xsl:sequence select="$expanded-clause/Node" />
            </NodeList>
          </NodeBody>
        </Node>
      </xsl:when>
      <xsl:otherwise>
        <!-- copy existing node and add conditions -->
        <Node NodeType="{$expanded-clause/Node/@NodeType}" epx="{name(.)}">
          <xsl:call-template name="standard-preamble">
            <xsl:with-param name="context" select="$expanded-clause/Node" />
          </xsl:call-template>
          <StartCondition>
            <xsl:sequence select="$start-condition" />
          </StartCondition>
          <SkipCondition>
            <xsl:sequence select="$skip-condition" />
          </SkipCondition>
          <xsl:sequence select="$expanded-clause/Node/NodeBody" />
        </Node>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="While">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="While">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:call-template name="while-body" />
    </Node>
  </xsl:template>

  <xsl:template name="while-body"> 
    <xsl:choose>
      <xsl:when test="VariableDeclarations|RepeatCondition|StartCondition">
        <!-- must create outer wrapper node -->
        <NodeBody>
          <NodeList>
            <Node NodeType="NodeList" epx="aux">
              <NodeId>
                <xsl:value-of select="tr:prefix('WhileBody')" />
              </NodeId>
              <xsl:call-template name="while-body-1" />
            </Node>
          </NodeList>
        </NodeBody>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="while-body-1" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="while-body-1"> 
    <xsl:variable name="test-id" select="tr:prefix('WhileTest')" />
    <RepeatCondition>
      <Succeeded>
        <NodeRef dir="child">
          <xsl:value-of select="$test-id" />
        </NodeRef>
      </Succeeded>
    </RepeatCondition>
    <NodeBody>
      <NodeList>
        <Node NodeType="Empty" epx="Condition">
          <NodeId>
            <xsl:value-of select="$test-id" />
          </NodeId>
          <PostCondition>
            <xsl:apply-templates select="Condition/*" />
          </PostCondition>
        </Node>
        <xsl:call-template name="while-body-2">
          <xsl:with-param name="test-id" select="$test-id"/>
        </xsl:call-template>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template name="while-body-2">
    <xsl:param name="test-id" required="yes" />
    <xsl:variable name="expanded-action">
      <xsl:apply-templates select="Action/*" />
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$expanded-action/Node/(StartCondition|SkipCondition)">
        <!-- must create wrapper node -->
        <Node NodeType="NodeList" epx="Action">
          <NodeId>
            <xsl:value-of select="tr:prefix('WhileAction')" />
          </NodeId>
          <xsl:call-template name="while-body-conds">
            <xsl:with-param name="test-id" select="$test-id" />
          </xsl:call-template>
          <NodeBody>
            <NodeList>
              <xsl:sequence select="$expanded-action/Node" />
            </NodeList>
          </NodeBody>
        </Node>
      </xsl:when>
      <xsl:otherwise>
        <!-- copy existing node and add conditions -->
        <Node NodeType="{$expanded-action/Node/@NodeType}" epx="Action">
          <xsl:call-template name="standard-preamble">
            <xsl:with-param name="context" select="$expanded-action/Node" />
          </xsl:call-template>
          <xsl:call-template name="while-body-conds">
            <xsl:with-param name="test-id" select="$test-id" />
          </xsl:call-template>
          <xsl:sequence select="$expanded-action/Node/NodeBody" />
        </Node>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="while-body-conds">
    <xsl:param name="test-id" required="yes" />
    <xsl:variable name="test-ref">
      <NodeRef dir="sibling">
        <xsl:value-of select="$test-id" />
      </NodeRef>
    </xsl:variable>
    <StartCondition>
      <Succeeded>
        <xsl:sequence select="$test-ref" />
      </Succeeded>
    </StartCondition>
    <SkipCondition>
      <PostconditionFailed>
        <xsl:sequence select="$test-ref" />
      </PostconditionFailed>
    </SkipCondition>
  </xsl:template>

  <xsl:template match="For">
    <xsl:param name="mode" />
    <Node NodeType="NodeList" epx="For">
      <xsl:call-template name="basic-clauses" />
      <xsl:call-template name="for-loop-variable-declarations" />
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <xsl:call-template name="for-body" />
    </Node>
  </xsl:template>

  <xsl:template name="for-body">
    <xsl:variable name="loop-node-id" select="tr:prefix('ForLoop')" />
    <xsl:variable name="do-node-id" select="tr:prefix('ForDo')" />
    <xsl:variable name="expanded-action">
      <xsl:apply-templates select="Action/*" />
    </xsl:variable>
    <NodeBody>
      <NodeList>
        <Node NodeType="NodeList" epx="aux">
          <NodeId>
            <xsl:value-of select="$loop-node-id" />
          </NodeId>
          <SkipCondition>
            <NOT>
              <xsl:apply-templates select="Condition/*" />
            </NOT>
          </SkipCondition>
          <RepeatCondition>
            <BooleanValue>true</BooleanValue>
          </RepeatCondition>
          <NodeBody>
            <NodeList>
              <xsl:sequence select="$expanded-action" />
              <Node NodeType="Assignment" epx="LoopVariableUpdate">
                <NodeId>
                  <xsl:value-of select="tr:prefix('ForLoopUpdater')" />
                </NodeId>
                <StartCondition>
                  <Finished>
                    <NodeRef dir="sibling">
                      <xsl:value-of select="$expanded-action/Node/NodeId" />
                    </NodeRef>
                  </Finished>
                </StartCondition>
                <NodeBody>
                  <Assignment>
                    <xsl:choose>
                      <xsl:when
                        test="LoopVariable/DeclareVariable/Type = 'Integer'">
                        <IntegerVariable>
                          <xsl:value-of
                            select="LoopVariable/DeclareVariable/Name" />
                        </IntegerVariable>
                      </xsl:when>
                      <xsl:when
                        test="LoopVariable/DeclareVariable/Type = 'Real'">
                        <RealVariable>
                          <xsl:value-of
                            select="LoopVariable/DeclareVariable/Name" />
                        </RealVariable>
                      </xsl:when>
                      <xsl:otherwise>
                        <error>Illegal loop variable type in For</error>
                      </xsl:otherwise>
                    </xsl:choose>
                    <NumericRHS>
                      <xsl:sequence select="LoopVariableUpdate/*" />
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

  <xsl:template name="for-loop-variable-declarations">
    <xsl:param name="context" select="." />
    <VariableDeclarations>
      <xsl:apply-templates select="VariableDeclarations/*"/>
      <xsl:sequence select="LoopVariable/*" />
    </VariableDeclarations>
  </xsl:template>
 

  <!-- Wait -->

  <xsl:template match="Wait">
    <xsl:param name="mode" />
    <Node NodeType="Empty" epx="Wait">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:choose>
        <xsl:when test="$mode='unordered'">
          <xsl:apply-templates select="StartCondition|RepeatCondition|
                                       PreCondition|PostCondition|
                                       InvariantCondition|ExitCondition|
                                       SkipCondition" />
        </xsl:when>
        <xsl:when test="$mode='ordered'">
          <xsl:call-template name="ordered-start-condition"/>
          <xsl:call-template name="ordered-skip-condition"/>
          <xsl:apply-templates select="RepeatCondition|
                                       PreCondition|PostCondition|
                                       InvariantCondition|ExitCondition" />
        </xsl:when>
      </xsl:choose>
      <xsl:call-template name="wait-end-condition" />
    </Node>
  </xsl:template>

  <xsl:template name="wait-end-condition">
    <xsl:variable name="timeout-test">
      <xsl:call-template name="timed-out">
        <xsl:with-param name="element" select="Units/*" />
      </xsl:call-template>
    </xsl:variable>
    <EndCondition>
      <xsl:choose>
        <xsl:when test="EndCondition">
          <OR>
            <xsl:apply-templates select="EndCondition/*" />
            <xsl:sequence select="$timeout-test" />
          </OR>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="$timeout-test" />
        </xsl:otherwise>
      </xsl:choose>
    </EndCondition>
  </xsl:template>

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
          <xsl:call-template name="insert-node-id"/>
          <NodeStateValue>EXECUTING</NodeStateValue>
          <Timepoint>START</Timepoint>
        </NodeTimepointValue>
      </ADD>
    </GE>
  </xsl:template>

  <xsl:template match="SynchronousCommand">
    <xsl:param name="mode" />
    <xsl:choose>
      <xsl:when test="Command/(IntegerVariable|RealVariable|
                      BooleanVariable|StringVariable|ArrayVariable)">
        <xsl:call-template name="command-with-return">
          <xsl:with-param name="mode" select="$mode" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="command-without-return">
          <xsl:with-param name="mode" select="$mode" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

<!-- NOTE: the following two templates could be refactored a bit! -->

  <xsl:template name="command-with-return">
    <xsl:param name="mode" />
    <xsl:variable name="return" select="tr:prefix('return')"/>
    <!-- Hack to save array name, iff command's return is an array -->
    <xsl:variable name="array_name" select="Command/ArrayVariable"/>
    <xsl:variable name="decl">
      <xsl:choose>
        <xsl:when test="Command/IntegerVariable">
          <IntegerVariable><xsl:value-of select="$return"/></IntegerVariable>
        </xsl:when>
        <xsl:when test="Command/RealVariable">
          <RealVariable><xsl:value-of select="$return"/></RealVariable>
        </xsl:when>
        <xsl:when test="Command/StringVariable">
          <StringVariable><xsl:value-of select="$return"/></StringVariable>
        </xsl:when>
        <xsl:when test="Command/BooleanVariable">
          <BooleanVariable><xsl:value-of select="$return"/></BooleanVariable>
        </xsl:when>
        <xsl:when test="Command/ArrayVariable">
          <ArrayVariable><xsl:value-of select="$return"/></ArrayVariable>
        </xsl:when>
        <xsl:otherwise>
          <error>Unrecognized variable type in SynchronousCommand</error>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="known-test">
      <xsl:choose>
        <xsl:when test="not(Command/ArrayVariable)">
          <IsKnown><xsl:sequence select="$decl"/></IsKnown>
        </xsl:when>
        <xsl:otherwise>
          <IsKnown>
            <ArrayElement>
              <Name><xsl:value-of select="$return"/></Name>
              <Index><IntegerValue>0</IntegerValue></Index>
          </ArrayElement>
          </IsKnown>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <Node NodeType="NodeList" epx="SynchronousCommand">
      <xsl:call-template name="standard-preamble">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <NodeBody>
        <NodeList>
          <Node NodeType="NodeList" epx="aux">
            <NodeId>
              <xsl:value-of select="tr:prefix('SynchronousCommandAux')" />
            </NodeId>
            <VariableDeclarations>
              <xsl:choose>
                <xsl:when test="Command/IntegerVariable">
                  <xsl:call-template name="declare-variable">
                    <xsl:with-param name="name" select="$return"/>
                    <xsl:with-param name="type" select="'Integer'"/>
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="Command/RealVariable">
                  <xsl:call-template name="declare-variable">
                    <xsl:with-param name="name" select="$return"/>
                    <xsl:with-param name="type" select="'Real'"/>
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="Command/StringVariable">
                  <xsl:call-template name="declare-variable">
                    <xsl:with-param name="name" select="$return"/>
                    <xsl:with-param name="type" select="'String'"/>
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="Command/BooleanVariable">
                  <xsl:call-template name="declare-variable">
                    <xsl:with-param name="name" select="$return"/>
                    <xsl:with-param name="type" select="'Boolean'"/>
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="Command/ArrayVariable">
                  <DeclareArray>
                    <Name><xsl:value-of select="$return"/></Name>
                    <!-- A royal hack!  Couldn't find a more compact expression that worked. -->
                    <xsl:choose>
                      <!-- First see if the array we are proxying is local -->
                      <xsl:when test="VariableDeclarations/DeclareArray[Name = $array_name][last()]">
                        <xsl:sequence select="VariableDeclarations/DeclareArray[Name = $array_name][last()]/Type"/>
                        <xsl:sequence select="VariableDeclarations/DeclareArray[Name = $array_name][last()]/MaxSize"/>
                        <!-- Otherwise find it in the closest ancestor -->
                      </xsl:when>
                      <xsl:when test="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]">
                        <xsl:sequence select="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]/Type"/>
                        <xsl:sequence select="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]/MaxSize"/>
                      </xsl:when>
                    </xsl:choose>
                  </DeclareArray>
                </xsl:when>
                <xsl:otherwise>
                  <error>Unrecognized variable type in SynchronousCommand</error>
                </xsl:otherwise>
              </xsl:choose>
            </VariableDeclarations>
            <xsl:if test="Timeout">
              <InvariantCondition>
                <xsl:call-template name="timed-out">
                  <xsl:with-param name="element" select="Timeout/*"/>
                </xsl:call-template>
              </InvariantCondition>
            </xsl:if>
            <NodeBody>
              <NodeList>
                <Node NodeType="Command" epx="aux">
                  <NodeId>
                    <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                  </NodeId>
                  <EndCondition>
                    <xsl:sequence select="$known-test"/>
                  </EndCondition>
                  <NodeBody>
                    <Command>
                      <xsl:sequence select="Command/ResourceList"/>
                      <xsl:sequence select="$decl"/>
                      <xsl:sequence select="Command/Name"/>
                      <xsl:sequence select="Command/Arguments"/>
                    </Command>
                  </NodeBody>
                </Node>
                <Node NodeType="Assignment" epx="aux">
                  <NodeId>
                    <xsl:value-of select="tr:prefix('SynchronousCommandAssignment')" />
                  </NodeId>
                  <StartCondition>
                    <Finished>
                      <NodeRef dir="sibling">
                        <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                      </NodeRef>
                    </Finished>
                  </StartCondition>
                  <NodeBody>
                    <Assignment>
                      <xsl:sequence select="Command/IntegerVariable|
                                            Command/RealVariable|
                                            Command/StringVariable|
                                            Command/BooleanVariable|
                                            Command/ArrayVariable"/>
                      <xsl:choose>
                        <xsl:when test="Command/IntegerVariable|
                                         Command/RealVariable">
                          <NumericRHS><xsl:sequence select="$decl"/></NumericRHS>
                        </xsl:when>
                        <xsl:when test="Command/StringVariable">
                          <StringRHS><xsl:sequence select="$decl"/></StringRHS>
                        </xsl:when>
                        <xsl:when test="Command/BooleanVariable">
                          <BooleanRHS><xsl:sequence select="$decl"/></BooleanRHS>
                        </xsl:when>
                        <xsl:when test="Command/ArrayVariable">
                          <ArrayRHS><xsl:sequence select="$decl"/></ArrayRHS>
                        </xsl:when>
                        <xsl:otherwise>
                          <error>Unrecognized variable type in SynchronousCommand</error>
                        </xsl:otherwise>
                      </xsl:choose>
                    </Assignment>
                  </NodeBody>
                </Node>
              </NodeList>
            </NodeBody>
          </Node>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="command-without-return">
    <xsl:param name="mode"/>
    <Node NodeType="NodeList" epx="SynchronousCommand">
      <xsl:call-template name="standard-preamble">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
      <NodeBody>
        <NodeList>
          <Node NodeType="NodeList" epx="aux">
            <NodeId>
              <xsl:value-of select="tr:prefix('SynchronousCommandAux')" />
            </NodeId>
            <xsl:if test="Timeout">
              <InvariantCondition>
                <xsl:call-template name="timed-out">
                  <xsl:with-param name="element" select="Timeout/*"/>
                </xsl:call-template>
              </InvariantCondition>
            </xsl:if>
            <NodeBody>
              <NodeList>
                <Node NodeType="Command" epx="aux">
                  <NodeId>
                    <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                  </NodeId>
                  <EndCondition>
                    <OR>
                      <EQInternal>
                        <NodeCommandHandleVariable>
                          <NodeId>
                            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                          </NodeId>
                        </NodeCommandHandleVariable>
                        <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                      </EQInternal>
                      <EQInternal>
                        <NodeCommandHandleVariable>
                          <NodeId>
                            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                          </NodeId>
                        </NodeCommandHandleVariable>
                        <NodeCommandHandleValue>COMMAND_FAILED</NodeCommandHandleValue>
                      </EQInternal>
                      <EQInternal>
                        <NodeCommandHandleVariable>
                          <NodeId>
                            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                          </NodeId>
                        </NodeCommandHandleVariable>
                        <NodeCommandHandleValue>COMMAND_DENIED</NodeCommandHandleValue>
                      </EQInternal>
                    </OR>
                  </EndCondition>
                  <NodeBody>
                    <Command>
                      <xsl:sequence select="Command/ResourceList"/>
                      <xsl:sequence select="Command/Name"/>
                      <xsl:sequence select="Command/Arguments"/>
                    </Command>
                  </NodeBody>
                </Node>
              </NodeList>
            </NodeBody>
          </Node>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="standard-preamble">
    <xsl:param name="context" select="." />
    <xsl:param name="mode" />
    <xsl:call-template name="basic-clauses">
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
    <xsl:apply-templates select="$context/VariableDeclarations" />
    <xsl:call-template name="translate-conditions">
      <xsl:with-param name="context" select="$context" />
      <xsl:with-param name="mode" select="$mode" />
    </xsl:call-template>
  </xsl:template>


  <!-- Action support -->

  <xsl:template name="basic-clauses">
    <xsl:param name="context" select="." />
    <!-- Copy attributes first -->
    <xsl:call-template name="copy-source-locator-attributes">
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
    <!-- Then handle NodeId -->
    <xsl:call-template name="insert-node-id">
      <xsl:with-param name="node" select="$context" />
    </xsl:call-template>
    <!-- Copy clauses that don't need translation -->
    <xsl:call-template name="handle-common-clauses" >
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="copy-source-locator-attributes">
    <xsl:param name="context" />
    <xsl:copy-of select="$context/(@FileName|@LineNo|@ColNo)" />
  </xsl:template>

  <xsl:template name="handle-common-clauses">
    <xsl:param name="context" />
    <xsl:sequence select="$context/(Comment|Priority)" />
    <xsl:apply-templates select="$context/Interface"/>
  </xsl:template>

  <xsl:template name="translate-conditions">
    <xsl:param name="context" select="." />
    <xsl:param name="mode" />
    <xsl:choose>
      <xsl:when test="$mode = 'ordered'">
        <xsl:call-template name="ordered-start-condition">
          <xsl:with-param name="context" select="$context" />
        </xsl:call-template>
        <xsl:call-template name="ordered-skip-condition">
          <xsl:with-param name="context" select="$context" />
        </xsl:call-template>
        <xsl:apply-templates
            select="$context/(RepeatCondition|PreCondition|
                    ExitCondition|InvariantCondition|
                    EndCondition|PostCondition)" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates
            select="$context/(StartCondition|SkipCondition|
                    RepeatCondition|PreCondition|
                    ExitCondition|InvariantCondition|
                    EndCondition|PostCondition)" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="ordered-start-condition">
    <xsl:param name="context" select="." />
    <xsl:choose>
      <xsl:when
          test="key('action', $context/preceding-sibling::element()[1])">
        <xsl:variable name="start-test">
          <xsl:call-template name="ordered-start-test">
            <xsl:with-param name="context" select="$context" />
          </xsl:call-template>
        </xsl:variable>
        <StartCondition>
          <xsl:choose>
            <xsl:when test="$context/StartCondition">
              <AND>
                <xsl:sequence select="$start-test" />
                <xsl:apply-templates select="$context/StartCondition/*" />
              </AND>
            </xsl:when>
            <xsl:when test="count($start-test/*) > 1">
              <AND>
                <xsl:sequence select="$start-test" />
              </AND>
            </xsl:when>
            <xsl:otherwise>
              <xsl:sequence select="$start-test" />
            </xsl:otherwise>
          </xsl:choose>
        </StartCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$context/StartCondition" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="ordered-start-test">
    <xsl:param name="context"/>
    <Finished>
      <NodeRef dir="sibling">
        <xsl:call-template name="node-id">
          <xsl:with-param name="context"
                          select="$context/preceding-sibling::*[1]" />
        </xsl:call-template>
      </NodeRef>
    </Finished>
  </xsl:template>
  
  <xsl:template name="ordered-skip-condition">
    <xsl:param name="context" select="." />
    <xsl:if test="SkipCondition">
      <xsl:choose>
        <xsl:when
            test="key('action', $context/preceding-sibling::element()[1])">
          <SkipCondition>
            <AND>
              <xsl:call-template name="ordered-skip-test">
                <xsl:with-param name="context" select="$context" />
              </xsl:call-template>
              <xsl:apply-templates select="$context/SkipCondition/*" />
            </AND>
          </SkipCondition>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="$context/SkipCondition" />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <xsl:template name="ordered-skip-test">
    <xsl:param name="context" />
    <Finished>
      <NodeRef dir="sibling">
        <xsl:call-template name="node-id">
          <xsl:with-param name="context"
                          select="$context/preceding-sibling::*[1]" />
        </xsl:call-template>
      </NodeRef>
    </Finished>
  </xsl:template>

  <xsl:template name="node-id">
    <xsl:param name="context" select="." />
    <xsl:choose>
      <xsl:when test="$context/NodeId">
        <xsl:value-of select="$context/NodeId" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="tr:node-id($context)" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="Interface">
	<Interface>
      <xsl:apply-templates select="In"/>	
      <xsl:apply-templates select="InOut"/>
	</Interface>
  </xsl:template>

  <xsl:template match="In">
	<In>
      <xsl:apply-templates select="*"/>
	</In>
  </xsl:template>

  <xsl:template match="InOut">
	<InOut>
    <xsl:apply-templates select="*"/>
	</InOut>
  </xsl:template>
  
  <xsl:template
      match="StartCondition|RepeatCondition|PreCondition|
             PostCondition|InvariantCondition|EndCondition|
             ExitCondition|SkipCondition">
    <xsl:element name="{name()}">
      <xsl:apply-templates select="*" />
    </xsl:element>
  </xsl:template>

  <xsl:template name="insert-node-id">
    <xsl:param name="node" select="." />
    <!-- Supply missing NodeId or copy existing one -->
    <xsl:choose>
      <xsl:when test="not($node/NodeId)">
        <NodeId>
          <xsl:value-of select="tr:node-id($node)" />
        </NodeId>
      </xsl:when>
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

  <!-- Boolean Expressions -->

  <!-- These expressions are translated recursively. -->
  <xsl:template match="OR|XOR|AND|NOT|
                       EQNumeric|EQInternal|EQString|EQBoolean|EQArray|
                       NENumeric|NEInternal|NEString|NEBoolean|NEArray|
                       GT|GE|LT|LE|
                       IsKnown|ALL_KNOWN|ANY_KNOWN">
    <xsl:element name="{name()}">
      <xsl:apply-templates select="*" />
    </xsl:element>
  </xsl:template>

  <!-- These expressions are deep copied. (But must also be processed
       for dates and durations) -->
  <xsl:template match="BooleanVariable|BooleanValue|LookupOnChange|LookupNow|ArrayElement">
    <xsl:copy>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>

  <!-- To be implemented as Core Plexil primitives -->

  <xsl:template match="IterationSucceeded">
    <AND>
      <IterationEnded>
        <xsl:sequence select="*" />
      </IterationEnded>
      <xsl:call-template name="noderef-succeeded" />
    </AND>
  </xsl:template>

  <xsl:template match="Interrupted">
    <xsl:param name="ref" select="*" />
    <AND>
      <Finished>
        <xsl:sequence select="$ref" />
      </Finished>
      <xsl:call-template name="noderef-interrupted">
        <xsl:with-param name="ref" select="$ref" />
      </xsl:call-template>
    </AND>
  </xsl:template>

  <xsl:template match="IterationFailed">
    <xsl:param name="ref" select="*" />
    <AND>
      <IterationEnded>
        <xsl:sequence select="$ref" />
      </IterationEnded>
      <xsl:call-template name="noderef-failed">
        <xsl:with-param name="ref" select="$ref" />
      </xsl:call-template>
    </AND>
  </xsl:template>

  <xsl:template match="InvariantFailed">
    <xsl:param name="ref" select="*" />
    <AND>
      <Finished>
        <xsl:sequence select="$ref"/>
      </Finished>
      <xsl:call-template name="noderef-invariant-failed">
        <xsl:with-param name="ref" select="$ref" />
      </xsl:call-template>
    </AND>
  </xsl:template>

  <xsl:template match="PreconditionFailed">
    <xsl:param name="ref" select="*" />
    <AND>
      <Finished>
        <xsl:sequence select="$ref"/>
      </Finished>
      <xsl:call-template name="noderef-precondition-failed">
        <xsl:with-param name="ref" select="$ref" />
      </xsl:call-template>
    </AND>
  </xsl:template>

  <xsl:template match="ParentFailed">
    <xsl:param name="ref" select="*" />
    <AND>
      <Finished>
        <xsl:sequence select="$ref"/>
      </Finished>
      <xsl:call-template name="noderef-parent-failed">
        <xsl:with-param name="ref" select="$ref" />
      </xsl:call-template>
    </AND>
  </xsl:template>

  <!--
    Support for message passing between executives
  -->

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

  <xsl:template match="OnMessage">
    <xsl:param name="mode" />
    <xsl:variable name="Msg_staging">
      <xsl:call-template name="OnMessage-staging" />
    </xsl:variable>
    <xsl:apply-templates select="$Msg_staging">
      <xsl:with-param name="mode" select="$mode" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template name="OnMessage-staging">
    <Sequence>
      <xsl:copy-of select="@FileName" />
      <xsl:copy-of select="@LineNo" />
      <xsl:copy-of select="@ColNo" />
      <VariableDeclarations>
        <DeclareVariable>
          <Name>
            <xsl:value-of select="tr:prefix('hdl')" />
          </Name>
          <Type>String</Type>
        </DeclareVariable>
      </VariableDeclarations>
      <xsl:copy-of select="NodeId" />
      <!-- Find parent node and set invariant, if exists -->
      <xsl:variable name="parent_id">
        <xsl:call-template name="parent-id-value" />
      </xsl:variable>
      <xsl:if test="not($parent_id='')">
        <InvariantCondition>
          <Executing>
            <NodeId>
              <xsl:value-of select="$parent_id" />
            </NodeId>
          </Executing>
        </InvariantCondition>
      </xsl:if>
      <!-- Msg wait node -->
      <xsl:variable name="hdl_dec">
        <StringVariable>
          <xsl:value-of select="tr:prefix('hdl')" />
        </StringVariable>
      </xsl:variable>
      <xsl:call-template name="run-wait-command">
        <xsl:with-param name="command" select="'ReceiveMessage'" />
        <xsl:with-param name="dest" select="$hdl_dec" />
        <xsl:with-param name="args" select="Message/*" />
      </xsl:call-template>
      <!-- Action for this message -->
      <Node NodeType="NodeList" epx="aux">
        <NodeId>
          <xsl:value-of
            select="concat(tr:prefix('MsgAction'), '_', Name/StringValue/text())" />
        </NodeId>
        <NodeBody>
          <NodeList>
            <xsl:sequence select="key('action', *)" />
          </NodeList>
        </NodeBody>
      </Node>
    </Sequence>
  </xsl:template>

  <xsl:template match="OnCommand">
    <xsl:param name="mode" />
    <xsl:variable name="Cmd_staging">
      <xsl:call-template name="OnCommand-staging">
        <xsl:with-param name="mode" select="$mode" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:apply-templates select="$Cmd_staging">
      <xsl:with-param name="mode" select="$mode" />
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template name="OnCommand-staging">
    <xsl:param name="mode"/>
    <Sequence>
      <xsl:copy-of select="@FileName" />
      <xsl:copy-of select="@LineNo" />
      <xsl:copy-of select="@ColNo" />
      <VariableDeclarations>
        <xsl:apply-templates select="VariableDeclarations/DeclareVariable"/>
        <!-- Arrays are variables too -->
        <xsl:apply-templates select="VariableDeclarations/DeclareArray"/>
        <DeclareVariable>
          <Name>
            <xsl:value-of select="tr:prefix('hdl')" />
          </Name>
          <Type>String</Type>
        </DeclareVariable>
      </VariableDeclarations>
      <!-- Handle the OnCommand node conditions -->
      <xsl:call-template name="translate-conditions">
        <xsl:with-param name="mode" select="$mode"/>
      </xsl:call-template>
      <!-- Find parent node and set invariant, if exists -->
      <xsl:variable name="parent_id">
        <xsl:call-template name="parent-id-value" />
      </xsl:variable>
      <!-- This invariant condition can create an out-of-scope node reference,
           e.g. when the OnCommand occurs inside a While.  The purpose and usefulness
           of this condition is questionable to begin with, so we're trying without
           it...
      <xsl:if test="not($parent_id='')">
        <InvariantCondition>
          <Executing>
            <NodeId>
              <xsl:value-of select="$parent_id" />
            </NodeId>
          </Executing>
        </InvariantCondition>
      </xsl:if> -->
      <xsl:sequence select="NodeId" />
      <!-- Cmd wait node -->
      <xsl:variable name="hdl_dec">
        <StringVariable>
          <xsl:value-of select="tr:prefix('hdl')" />
        </StringVariable>
      </xsl:variable>
      <xsl:variable name="arg_dec">
        <StringValue>
          <xsl:value-of select="Name/StringValue" />
        </StringValue>
      </xsl:variable>
      <xsl:call-template name="run-wait-command">
        <xsl:with-param name="command" select="'ReceiveCommand'" />
        <xsl:with-param name="dest" select="$hdl_dec" />
        <xsl:with-param name="args" select="$arg_dec" />
      </xsl:call-template>
      <!-- Cmd get parameters nodes -->
      <xsl:for-each select="VariableDeclarations/DeclareVariable | VariableDeclarations/DeclareArray">
        <Node NodeType="Command" epx="aux">
          <NodeId>
            <xsl:value-of
              select="concat(tr:prefix('CmdGetParam'), '_', Name/text())" />
          </NodeId>
          <EndCondition>
            <IsKnown>
              <xsl:choose>
                <xsl:when test="MaxSize"> <!-- Arrays -->
                  <ArrayElement>
                    <Name><xsl:value-of select="Name"/></Name>
                    <Index><IntegerValue>0</IntegerValue></Index>
                  </ArrayElement>
                </xsl:when>
                <xsl:otherwise> <!-- Scalars -->
                  <xsl:element name='{concat(Type/text(), "Variable")}'>
                    <xsl:value-of select="Name/text()" />
                  </xsl:element>
                </xsl:otherwise>
              </xsl:choose>
            </IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <xsl:choose>
                <xsl:when test="MaxSize"> <!-- Arrays -->
                  <ArrayVariable><xsl:value-of select="Name"/></ArrayVariable>
                </xsl:when>
                <xsl:otherwise> <!-- Scalars -->
                  <xsl:element name='{concat(Type/text(), "Variable")}'>
                    <xsl:value-of select="Name/text()" />
                  </xsl:element>
                </xsl:otherwise>
              </xsl:choose>
              <Name>
                <StringValue>GetParameter</StringValue>
              </Name>
              <Arguments>
                <StringVariable>
                  <xsl:value-of select="tr:prefix('hdl')" />
                </StringVariable>
                <IntegerValue>
                  <xsl:value-of select="position() - 1" />
                </IntegerValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
      </xsl:for-each>
      <!-- Action for this command -->
      <Node NodeType="NodeList" epx="aux">
        <NodeId>
          <xsl:value-of
            select="concat(tr:prefix('CmdAction'), '_', Name/StringValue/text())" />
        </NodeId>
        <NodeBody>
          <NodeList>
            <xsl:apply-templates select="key('action', *)"
              mode="oncommand-mode" />
          </NodeList>
        </NodeBody>
      </Node>
      <!--  Insert return value command if not present -->
      <xsl:if
        test="not(.//Command/Name/StringValue/text() = 'SendReturnValue')">
        <Node NodeType="Command" epx="aux">
          <NodeId>
            <xsl:value-of select="tr:prefix('CmdReturn')" />
          </NodeId>
          <NodeBody>
            <Command>
              <Name>
                <StringValue>SendReturnValue</StringValue>
              </Name>
              <Arguments>
                <StringVariable>
                  <xsl:value-of select="tr:prefix('hdl')" />
                </StringVariable>
                <BooleanValue>true</BooleanValue>
              </Arguments>
            </Command>
          </NodeBody>
        </Node>
      </xsl:if>
    </Sequence>
  </xsl:template>

  <!--
    Recursive template that attempts to find and insert the NodeId of
    the parent action to the given node
  -->
  <xsl:template name="parent-id-value">
    <xsl:param name="start_path" select="." />
    <xsl:choose>
      <!-- Insert NodeId via insert-node-id template -->
      <xsl:when test="key('action', $start_path/..)">
        <xsl:variable name="id">
          <xsl:call-template name="insert-node-id">
            <xsl:with-param name="node" select="$start_path/.." />
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


  <xsl:template name="run-wait-command">
    <xsl:param name="command" />
    <xsl:param name="dest" />
    <xsl:param name="args" />
    <Node NodeType="Command" epx="aux">
      <NodeId>
        <xsl:copy-of select="tr:prefix('CmdWait')" />
      </NodeId>
      <EndCondition>
        <IsKnown>
          <xsl:sequence select="$dest" />
        </IsKnown>
      </EndCondition>
      <NodeBody>
        <Command>
          <xsl:sequence select="$dest" />
          <Name>
            <StringValue>
              <xsl:sequence select="$command" />
            </StringValue>
          </Name>
          <Arguments>
            <xsl:sequence select="$args" />
          </Arguments>
        </Command>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template match="Command" mode="oncommand-mode">
    <Command>
      <xsl:sequence select="node()[./local-name()!='Arguments'] " />
      <xsl:choose>
        <xsl:when test="Name/StringValue/text()='SendReturnValue'">
          <Arguments>
            <StringVariable>
              <xsl:value-of select="tr:prefix('hdl')" />
            </StringVariable>
            <xsl:sequence select="Arguments/node()" />
          </Arguments>
        </xsl:when>
        <xsl:otherwise>
          <xsl:sequence select="Arguments" />
        </xsl:otherwise>
      </xsl:choose>
    </Command>
  </xsl:template>

  <xsl:template match="*" mode="oncommand-mode">
    <xsl:copy>
      <xsl:copy-of select="@*" />
      <xsl:apply-templates mode="oncommand-mode" />
    </xsl:copy>
  </xsl:template>

  <!-- Node state/outcome/failure tests -->

  <!-- Node outcome checks -->

  <xsl:template name="noderef-outcome-check">
    <xsl:param name="ref" required="yes" />
    <xsl:param name="outcome" required="yes" />
    <EQInternal>
      <NodeOutcomeVariable>
        <xsl:sequence select="$ref" />
      </NodeOutcomeVariable>
      <NodeOutcomeValue>
        <xsl:value-of select="$outcome" />
      </NodeOutcomeValue>
    </EQInternal>
  </xsl:template>

  <xsl:template name="noderef-succeeded">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-outcome-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="outcome" select="'SUCCESS'" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="noderef-failed">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-outcome-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="outcome" select="'FAILURE'" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="noderef-interrupted">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-outcome-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="outcome" select="'INTERRUPTED'" />
    </xsl:call-template>
  </xsl:template>

  <!-- Node failure checks -->

  <xsl:template name="noderef-failure-check">
    <xsl:param name="ref" required="yes" />
    <xsl:param name="failure" required="yes" />
    <EQInternal>
      <NodeFailureVariable>
        <xsl:sequence select="$ref" />
      </NodeFailureVariable>
      <NodeFailureValue>
        <xsl:value-of select="$failure" />
      </NodeFailureValue>
    </EQInternal>
  </xsl:template>

  <xsl:template name="noderef-invariant-failed">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-failure-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="failure" select="'INVARIANT_CONDITION_FAILED'" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="noderef-precondition-failed">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-failure-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="failure" select="'PRE_CONDITION_FAILED'" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="noderef-parent-failed">
    <xsl:param name="ref" select="*" />
    <xsl:call-template name="noderef-failure-check">
      <xsl:with-param name="ref" select="$ref" />
      <xsl:with-param name="failure" select="'PARENT_FAILED'" />
    </xsl:call-template>
  </xsl:template>

  <!-- Generic Lookup form -->
  <xsl:template match="Lookup">
    <xsl:choose>
      <xsl:when test="ancestor::Command|ancestor::Assignment|ancestor::Update|
                      ancestor::Command|ancestor::PreCondition|ancestor::Condition|
                      ancestor::PostCondition|ancestor::InvariantCondition">
        <LookupNow>
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
        <LookupOnChange>
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


  <!-- Dates and Durations (handled rather naively as real values) -->

  <!-- Some Epochs http://en.wikipedia.org/wiki/Epoch_(reference_date) -->

  <!-- Julian day 2415021 (DJD+1?) -->
  <xsl:variable name="UTC" select="xs:dateTime('1900-01-01T00:00:00Z')"/>

  <!-- Julian day 2440587.5 -->
  <xsl:variable name="Unix" select="xs:dateTime('1970-01-01T00:00:00Z')"/>

  <!-- Julian day 2448250 -->
  <xsl:variable name="CPS" select="xs:dateTime('1990-12-24T00:00:00Z')"/>

  <!-- Julian date 2451545.0 TT -->
  <xsl:variable name="J2000" select="xs:dateTime('2000-01-01T11:58:55.816Z')"/>

  <!-- Use this epoch -->
  <xsl:variable name="epoch" select="$Unix"/>

  <xsl:template match="Type[.='Date' or .='Duration']">
    <!-- Dates and Durations are represented as "real" values -->
    <Type>Real</Type>
  </xsl:template>

  <xsl:template match="DurationVariable|DateVariable">
    <!-- Dates and Durations are represented as "real" variables -->
    <RealVariable><xsl:value-of select="."/></RealVariable>
  </xsl:template>

  <xsl:template match="DateValue">
    <!-- A Date is the number of seconds since the start of the epoch used on this platform -->
    <RealValue>
      <xsl:value-of select="tr:seconds(xs:dateTime(.) - xs:dateTime($epoch))"/>
    </RealValue>
  </xsl:template>

  <xsl:template match="DurationValue">
    <!-- A Duration is the number of seconds in the ISO 8601 duration -->
    <RealValue><xsl:value-of select="tr:seconds(.)"/></RealValue>
  </xsl:template>

  <!-- Return the (total) number of seconds in and ISO 8601 duration -->
  <xsl:function name="tr:seconds">
    <xsl:param name="duration"/>
    <xsl:value-of select="xs:dayTimeDuration($duration) div xs:dayTimeDuration('PT1.0S')"/>
  </xsl:function>

  <!-- Functions -->

  <!-- Computes a unique NodeID -->
  <xsl:function name="tr:node-id">
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
