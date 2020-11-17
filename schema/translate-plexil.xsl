<?xml version="1.0" encoding="ISO-8859-1"?>

<!--
* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

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

  <xsl:template match="Node">
    <Node>
      <!-- Parts that are copied directly -->
      <xsl:copy-of select="@NodeType" />
      <xsl:call-template name="standard-preamble" />
      <xsl:apply-templates select="NodeBody" />
    </Node>
  </xsl:template>

  <!-- Sequence, CheckedSequence, UncheckedSequence -->
  <!-- We may switch Sequence to be an UncheckedSequence
       in future releases -->

  <xsl:template match="UncheckedSequence">
    <xsl:call-template name="UncheckedSequence" />
  </xsl:template>

  <xsl:template name="UncheckedSequence">
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="standard-preamble" />
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template match="Sequence|CheckedSequence">
    <xsl:call-template name="CheckedSequence" />
  </xsl:template>

  <xsl:template name="CheckedSequence">
    <xsl:variable name="conds">
      <xsl:call-template name="translate-conditions" />
    </xsl:variable>
    
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:sequence
          select="$conds/(* except InvariantCondition)" />
      <xsl:call-template name="success-invariant-condition" >
        <xsl:with-param name="expanded-invariant"
                        select="$conds/InvariantCondition" />
      </xsl:call-template>
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template name="success-invariant-condition">
    <xsl:param name="expanded-invariant" />
    <InvariantCondition>
      <xsl:choose>
        <xsl:when test="$expanded-invariant">
          <AND>
            <xsl:sequence select="$expanded-invariant/*" />
            <NoChildFailed> <NodeRef dir="self" /> </NoChildFailed>
          </AND>
        </xsl:when>
        <xsl:otherwise>
          <NoChildFailed> <NodeRef dir="self" /> </NoChildFailed> 
        </xsl:otherwise>
      </xsl:choose>
    </InvariantCondition>
  </xsl:template>

  <!-- Sequence guts -->

  <!-- Handle sequences in two steps: 
       1. Expand children. Generate node IDs as required.
       2. Generate start and skip conditions to enforce the sequence.
  -->

  <!-- The individual pieces of this are broken out separately
       because some macro expansions need to get information
       from the expanded child actions before constructing the body.
  -->

  <xsl:template name="sequence-body">
    <xsl:call-template name="link-sequence-children">
      <xsl:with-param name="children">
        <xsl:call-template name="expand-child-actions" />
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <!-- Useful in a number of templates -->
  <xsl:template name="expand-child-actions">
    <xsl:apply-templates select="tr:actions(.)" />
  </xsl:template>

  <xsl:template name="link-sequence-children">
    <xsl:param name="children" />
    <NodeBody>
      <NodeList>
        <xsl:for-each select="$children/*">
          <xsl:call-template name="sequence-linkage" />
        </xsl:for-each>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <!-- Called only on Node elements
       Node is expected to have a NodeId -->

  <xsl:template name="sequence-linkage">
    <Node>
      <xsl:copy-of select="@*"/>
      <!-- Make an attempt to preserve ordering -->
      <xsl:sequence select="NodeId|Comment|Priority|Interface|
                            VariableDeclarations|Assume|Desire|Expect" />
      <xsl:call-template name="ordered-start-condition" />
      <xsl:call-template name="ordered-skip-condition" />
      <xsl:sequence
          select="EndCondition|ExitCondition|InvariantCondition|
                  PostCondition|PreCondition|RepeatCondition|NodeBody"/>
    </Node>
  </xsl:template>

  <!-- Templates used only in sequence-linkage -->
  <!-- Note that each is called with a pre-expanded Node as its context
       and therefore need not worry whether its preceding sibling is an action.
  -->

  <xsl:template name="ordered-start-condition">
    <xsl:param name="context" select="." />
    <xsl:choose>
      <xsl:when test="$context/preceding-sibling::*[1]">
        <xsl:variable name="start-test">
          <Finished>
            <NodeRef dir="sibling">
              <xsl:call-template name="node-id">
                <xsl:with-param name="context"
                                select="$context/preceding-sibling::*[1]" />
              </xsl:call-template>
            </NodeRef>
          </Finished>
        </xsl:variable>
        <xsl:choose>
          <xsl:when test="$context/StartCondition">
            <StartCondition>
              <AND>
                <xsl:sequence select="$start-test" />
                <xsl:sequence select="$context/StartCondition/*" />
              </AND>
            </StartCondition>
          </xsl:when>
          <xsl:otherwise>
            <StartCondition>
              <xsl:sequence select="$start-test" />
            </StartCondition>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$context/StartCondition" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template name="ordered-skip-condition">
    <xsl:param name="context" select="." />
    <xsl:if test="$context/SkipCondition">
      <xsl:choose>
        <xsl:when test="$context/preceding-sibling::*[1]">
          <SkipCondition>
            <AND>
              <Finished>
                <NodeRef dir="sibling">
                  <xsl:call-template name="node-id">
                    <xsl:with-param name="context"
                                    select="$context/preceding-sibling::*[1]" />
                  </xsl:call-template>
                </NodeRef>
              </Finished>
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

  <!-- Concurrence -->

  <xsl:template match="Concurrence">
    <Node NodeType="NodeList" epx="Concurrence">
      <xsl:call-template name="standard-preamble" />
      <NodeBody>
        <NodeList>
          <xsl:call-template name="expand-child-actions" />
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- Try (not to be confused with try/catch) -->


  <xsl:template match="Try">
    <xsl:variable name="conds">
      <xsl:call-template name="translate-conditions" />
    </xsl:variable>
    <xsl:variable name="expanded-children">
      <xsl:call-template name="expand-child-actions" />
    </xsl:variable>

    <Node NodeType="NodeList" epx="Try">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:sequence select="$conds/(* except EndCondition|PostCondition)" />

      <xsl:call-template name="try-end-condition">
        <xsl:with-param name="original-condition" select="$conds/EndCondition"/>
        <xsl:with-param name="children" select="$expanded-children" />
      </xsl:call-template>
      <xsl:call-template name="try-post-condition" >
        <xsl:with-param name="original-condition" select="$conds/PostCondition"/>
        <xsl:with-param name="children" select="$expanded-children" />
      </xsl:call-template>

      <xsl:call-template name="link-sequence-children" >
        <xsl:with-param name="children" select="$expanded-children" />
      </xsl:call-template>
    </Node>
  </xsl:template>

  <xsl:template name="try-end-condition">
    <xsl:param name="original-condition" />
    <xsl:param name="children" />
    <xsl:choose>
      <xsl:when test="count($children/*) lt 2">
        <!-- No test needed, default all-children-finished is
             sufficient -->
        <xsl:sequence select="$original-condition" />
      </xsl:when>
      <xsl:otherwise>
        <EndCondition>
          <OR>
            <xsl:sequence select="$original-condition" />
            <xsl:for-each select="$children/*">
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
    <xsl:param name="original-condition" />
    <xsl:param name="children"/>

    <xsl:variable name="post-tests">
      <xsl:for-each select="$children/*">
        <Succeeded>
          <NodeRef dir="child">
            <xsl:call-template name="node-id" />
          </NodeRef>
        </Succeeded>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="post-test">
      <xsl:choose>
        <xsl:when test="count($post-tests/*) gt 1">
          <OR>
            <xsl:sequence select="$post-tests" />
          </OR>
        </xsl:when>
        <xsl:when test="count($post-tests/*) = 1">
          <xsl:sequence select="$post-tests" />
        </xsl:when>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="$original-condition or $post-test">
      <PostCondition>
        <xsl:choose>
          <xsl:when test="$original-condition and $post-test">
            <AND>
              <xsl:sequence select="$original-condition" />
              <xsl:sequence select="$post-test" />
            </AND>
          </xsl:when>
          <xsl:when test="PostCondition">
            <xsl:sequence select="$original-condition" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:sequence select="$post-test" />
          </xsl:otherwise>
        </xsl:choose>
      </PostCondition>
    </xsl:if>
  </xsl:template>

  <!-- If-Then-Elseif-Else -->

  <xsl:template match="If">
    <xsl:choose>
      <!-- General case -->
      <xsl:when test="ElseIf">
        <xsl:call-template name="generalized-if" />
      </xsl:when>

      <!-- If-Then-Else -->
      <!-- Requires a wrapper node, but can skip the test node. -->
      <xsl:when test="Else">
        <xsl:call-template name="basic-if-then-else" />
      </xsl:when>

      <!-- If-Then -->
      <xsl:otherwise>
        <xsl:call-template name="basic-if-then" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- TODO: figure out when we can collapse simple if-then to a single node -->

  <xsl:template name="basic-if-then">
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="standard-preamble" />
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
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="standard-preamble" />
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
    <Node NodeType="NodeList" epx="If">
      <xsl:call-template name="standard-preamble" />
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
          <NodeId generated="1">
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

  <xsl:template name="if-clause-body">
    <xsl:param name="start-condition" required="yes" />
    <xsl:param name="skip-condition" required="yes" />
    <xsl:variable name="expanded-clause">
      <xsl:apply-templates /> <!-- WARNING: NOT select='.', only apply to children of . -->
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$expanded-clause/Node/(StartCondition|SkipCondition)">
        <!-- must create wrapper node -->
        <Node NodeType="NodeList" epx="{name(.)}">
          <NodeId generated="1"><xsl:value-of select="tr:prefix(name(.))" /></NodeId>
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

  <xsl:template name="elseif-test-node-id">
    <xsl:param name="element" select="." />
    <xsl:value-of select="tr:prefix('ElseIf-')" />
    <xsl:value-of
        select="count($element/preceding-sibling::ElseIf) + 1" />
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
      <NodeId generated="1">
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


  <!-- ****************** -->
  <!-- Looping constructs -->
  <!-- ****************** -->

  <!-- while loop -->

  <xsl:template match="While">
    <Node NodeType="NodeList" epx="While">
      <xsl:call-template name="standard-preamble" />
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
              <NodeId generated="1">
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
          <NodeId generated="1">
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
          <NodeId generated="1">
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

  <!-- Do-while loop -->

  <!-- FIXME: Identify generated node IDs -->
  <xsl:template match="Do">
    <xsl:variable name="expanded-conditions" as="element()*">
      <xsl:call-template name="translate-conditions" />
    </xsl:variable>
    <xsl:variable name="expanded-declarations" as="element()*">
      <xsl:apply-templates select="VariableDeclarations" />
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$expanded-declarations or $expanded-conditions">
        <!-- must create outer wrapper node -->
        <Node NodeType="NodeList" epx="DoWrapper">
          <xsl:call-template name="basic-clauses" />
          <xsl:sequence select="$expanded-declarations" />
          <xsl:sequence select="$expanded-conditions" />
          <NodeBody>
            <NodeList>
              <xsl:call-template name="do-body" >
                <xsl:with-param name="node-id" />
              </xsl:call-template>
            </NodeList>
          </NodeBody>
        </Node>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="do-body">
          <xsl:with-param name="node-id" select="NodeId/text()" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="do-body"> 
    <!-- May be empty -->
    <xsl:param name="node-id"/>
    <!-- Action must be a single action -->
    <xsl:variable name="expanded-action">
      <xsl:apply-templates select="fn:exactly-one(Action/*)" />
    </xsl:variable>
    <xsl:choose>
      <!-- Must wrap any expansion which contains a RepeatCondition -->
      <xsl:when test="$expanded-action/Node/RepeatCondition">
        <xsl:call-template name="do-body-wrapper">
          <xsl:with-param name="node-id" select="$node-id" />
          <xsl:with-param name="expanded-action" select="$expanded-action" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <!-- Simple (sorta) case: hoist the expanded node up and tweak it a bit -->
        <Node epx="Do">
          <xsl:attribute name="NodeType" select="$expanded-action/Node/@NodeType" />
          <!-- FIXME: identify automatically generated node IDs -->
          <NodeId>
            <xsl:choose>
              <!-- Prefer NodeId passed in from above, if any -->
              <xsl:when test="$node-id">
                <xsl:value-of select="$node-id" />
              </xsl:when>
              <!-- Hoist up NodeId from expansion, if any -->
              <xsl:when test="$expanded-action/Node/NodeId/text()">
                <xsl:value-of select="$expanded-action/Node/NodeId/text()" />
              </xsl:when>
              <!-- Else gensym one -->
              <xsl:otherwise>
                <xsl:value-of select="tr:prefix('DoBody')" />
              </xsl:otherwise>
            </xsl:choose>
          </NodeId>
          <xsl:call-template name="handle-common-clauses" />
          <RepeatCondition>
            <xsl:apply-templates select="Condition/*" />
          </RepeatCondition>
          <xsl:sequence select="$expanded-action/Node/(* except NodeId)" />
        </Node>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="do-body-wrapper">
    <xsl:param name="node-id" />
    <xsl:param name="expanded-action" />
    <Node epx="Do" NodeType="ListNode">
      <NodeId> <xsl:value-of select="$node-id" /> </NodeId>
      <RepeatCondition>
        <xsl:apply-templates select="Condition/*" />
      </RepeatCondition>
      <NodeBody>
        <xsl:sequence select="$expanded-action" />
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- for loop -->

  <xsl:template match="For">
    <Node NodeType="NodeList" epx="For">
      <xsl:call-template name="basic-clauses" />
      <xsl:call-template name="for-loop-variable-declarations" />
      <xsl:call-template name="translate-conditions" />
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
          <NodeId generated="1">
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
                <NodeId generated="1">
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
    <xsl:variable name="conds">
      <xsl:call-template name="translate-conditions" />
    </xsl:variable>

    <Node NodeType="Empty" epx="Wait">
      <xsl:call-template name="basic-clauses" />
      <xsl:apply-templates select="VariableDeclarations" />
      <xsl:sequence select="$conds/(* except EndCondition)" />
      <xsl:call-template name="wait-end-condition" >
        <xsl:with-param name="original-condition"
                        select="$conds/EndCondition" />
      </xsl:call-template>
    </Node>
  </xsl:template>

  <xsl:template name="wait-end-condition">
    <xsl:param name="original-condition" />
    <xsl:variable name="timeout-test">
      <xsl:call-template name="timed-out">
        <xsl:with-param name="element" select="Units/*" />
      </xsl:call-template>
    </xsl:variable>
    <EndCondition>
      <xsl:choose>
        <xsl:when test="$original-condition">
          <OR>
            <xsl:sequence select="original-condition/*" />
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
          <NodeId><xsl:call-template name="node-id"/></NodeId>
          <NodeStateValue>EXECUTING</NodeStateValue>
          <Timepoint>START</Timepoint>
        </NodeTimepointValue>
      </ADD>
    </GE>
  </xsl:template>

  <!-- SynchronousCommand -->

  <xsl:template match="SynchronousCommand">
    <Node NodeType="NodeList" epx="SynchronousCommand">
      <xsl:call-template name="standard-preamble" />
      <NodeBody>
        <NodeList>
          <xsl:choose>
            <xsl:when test="Command/*[fn:ends-with(fn:name(), 'Variable')]">
              <xsl:call-template name="command-with-return" />
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name="command-without-return" />
            </xsl:otherwise>
          </xsl:choose>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="command-with-return">
    <Node NodeType="NodeList" epx="aux">
      <NodeId generated="1">
        <xsl:value-of select="tr:prefix('SynchronousCommandAux')" />
      </NodeId>
      <xsl:choose>
        <xsl:when test="Command/ArrayVariable">
          <xsl:call-template name="command-with-array-return-body" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="command-with-scalar-return-body" />
        </xsl:otherwise>
      </xsl:choose>
    </Node>
  </xsl:template>

  <xsl:template name="command-with-scalar-return-body">
    <xsl:variable name="return" select="tr:prefix('return')"/>
    <xsl:variable name="orig-ref"
                  select="Command/(BooleanVariable|IntegerVariable|RealVariable|StringVariable)" />
    <xsl:variable name="var-type" select="fn:substring-before(name($orig-ref), 'Variable')" />
    <xsl:variable name="ref">
      <xsl:element name="{name($orig-ref)}">
        <xsl:value-of select="$return"/>
      </xsl:element>
    </xsl:variable>

    <VariableDeclarations>
      <xsl:call-template name="declare-variable">
        <xsl:with-param name="name" select="$return"/>
        <xsl:with-param name="type" select="$var-type"/>
      </xsl:call-template>
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
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
          </NodeId>
          <EndCondition>
            <IsKnown><xsl:sequence select="$ref"/></IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <xsl:sequence select="Command/ResourceList"/>
              <xsl:sequence select="$ref"/>
              <xsl:sequence select="Command/Name"/>
              <xsl:sequence select="Command/Arguments"/>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment" epx="aux">
          <NodeId generated="1">
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
              <xsl:sequence select="$orig-ref"/>
              <xsl:choose>
                <xsl:when test="Command/BooleanVariable">
                  <BooleanRHS><xsl:sequence select="$ref"/></BooleanRHS>
                </xsl:when>
                <xsl:when test="Command/StringVariable">
                  <StringRHS><xsl:sequence select="$ref"/></StringRHS>
                </xsl:when>
                <xsl:otherwise>
                  <NumericRHS><xsl:sequence select="$ref"/></NumericRHS>
                </xsl:otherwise>
              </xsl:choose>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template name="command-with-array-return-body">
    <xsl:variable name="return" select="tr:prefix('return')"/>
    <xsl:variable name="array_name" select="Command/ArrayVariable"/>
    <xsl:variable name="ref">
      <ArrayVariable> <xsl:value-of select="$return"/> </ArrayVariable>
    </xsl:variable>

    <VariableDeclarations>
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
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
          </NodeId>
          <EndCondition>
            <IsKnown>
              <ArrayElement>
                <xsl:sequence select="$ref"/>
                <Index><IntegerValue>0</IntegerValue></Index>
              </ArrayElement>
            </IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <xsl:sequence select="Command/ResourceList"/>
              <xsl:sequence select="$ref"/>
              <xsl:sequence select="Command/Name"/>
              <xsl:sequence select="Command/Arguments"/>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment" epx="aux">
          <NodeId generated="1">
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
              <xsl:sequence select="$array_name"/>
              <ArrayRHS><xsl:sequence select="$ref"/></ArrayRHS>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </xsl:template>
  
  <xsl:template name="command-without-return">
    <Node NodeType="NodeList" epx="aux">
      <NodeId generated="1">
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
            <NodeId generated="1">
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
  </xsl:template>

  <!-- Processing for the standard parts of an action -->

  <xsl:template name="standard-preamble">
    <xsl:param name="context" select="." />
    <xsl:call-template name="basic-clauses">
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
    <xsl:apply-templates select="$context/VariableDeclarations" />
    <xsl:call-template name="translate-conditions">
      <xsl:with-param name="context" select="$context" />
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
    <!-- THIS SHOULD BE THE ONLY CALLER OF ensure-node-id -->
    <xsl:call-template name="ensure-node-id">
      <xsl:with-param name="node" select="$context" />
    </xsl:call-template>
    <!-- Copy clauses that don't need translation -->
    <xsl:call-template name="handle-common-clauses" >
      <xsl:with-param name="context" select="$context" />
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="copy-source-locator-attributes">
    <xsl:param name="context" select="." />
    <xsl:copy-of select="$context/(@FileName|@LineNo|@ColNo)" />
  </xsl:template>

  <xsl:template name="handle-common-clauses">
    <xsl:param name="context" select="." />
    <xsl:sequence select="$context/(Comment|Priority)" />
    <xsl:apply-templates select="$context/Interface"/>
    <xsl:apply-templates select="$context/UsingMutex" />
    <xsl:copy-of select="$context/(Assume|Desire|Expect)" />
  </xsl:template>

  <!-- This template should NEVER generate a new node ID. -->
  <xsl:template name="node-id">
    <xsl:param name="context" select="." />
    <xsl:value-of select="$context/NodeId" />
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

  <xsl:template name="translate-conditions">
    <xsl:param name="context" select="." />
    <xsl:apply-templates select="tr:conditions($context)" />
  </xsl:template>
  
  <xsl:template
      match="EndCondition|ExitCondition|InvariantCondition|
             PostCondition|PreCondition|RepeatCondition|
             SkipCondition|StartCondition">
    <xsl:element name="{name()}">
      <xsl:apply-templates select="*" /> <!-- I _think_ this is redundant -->
    </xsl:element>
  </xsl:template>

  <xsl:template name="ensure-node-id">
    <xsl:param name="node" select="." />
    <!-- Supply missing NodeId or copy existing one -->
    <xsl:choose>
      <xsl:when test="not($node/NodeId)">
        <NodeId generated="1">
          <xsl:value-of select="tr:gen-node-id($node)" />
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
      <xsl:apply-templates /> <!-- WARNING: select='.' will cause infinite loop here -->
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
    <xsl:variable name="Msg_staging">
      <xsl:call-template name="OnMessage-staging" />
    </xsl:variable>
    <xsl:apply-templates select="$Msg_staging" />
  </xsl:template>

  <xsl:template name="OnMessage-staging">
    <Sequence>
      <xsl:call-template name="basic-clauses" />
      <VariableDeclarations>
        <DeclareVariable>
          <Name>
            <xsl:value-of select="tr:prefix('hdl')" />
          </Name>
          <Type>String</Type>
        </DeclareVariable>
      </VariableDeclarations>
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
        <NodeId generated="1">
          <xsl:value-of
            select="concat(tr:prefix('MsgAction'), '_', Name/StringValue/text())" />
        </NodeId>
        <NodeBody>
          <NodeList>
            <xsl:call-template name="expand-child-actions" />
          </NodeList>
        </NodeBody>
      </Node>
    </Sequence>
  </xsl:template>

  <xsl:template match="OnCommand">
    <xsl:variable name="Cmd_staging">
      <xsl:call-template name="OnCommand-staging" />
    </xsl:variable>
    <xsl:apply-templates select="$Cmd_staging" />
  </xsl:template>

  <xsl:template name="OnCommand-staging">
    <Sequence>
      <xsl:call-template name="basic-clauses" />
      <VariableDeclarations>
        <xsl:apply-templates select="VariableDeclarations/DeclareVariable" />
        <!-- Arrays and mutexes are variables too -->
        <xsl:apply-templates select="VariableDeclarations/DeclareArray" />
        <xsl:apply-templates select="VariableDeclarations/DeclareMutex" />
        <DeclareVariable>
          <Name>
            <xsl:value-of select="tr:prefix('hdl')" />
          </Name>
          <Type>String</Type>
        </DeclareVariable>
      </VariableDeclarations>
      <!-- Handle the OnCommand node conditions -->
      <xsl:call-template name="translate-conditions" />
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
          <NodeId generated="1">
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
        <NodeId generated="1">
          <xsl:value-of
            select="concat(tr:prefix('CmdAction'), '_', Name/StringValue/text())" />
        </NodeId>
        <NodeBody>
          <NodeList>
            <xsl:for-each select="tr:actions(.)">
              <xsl:choose>
                <xsl:when test="Command">
                  <xsl:call-template name="on-command-command" />
                </xsl:when>
                <xsl:otherwise>
                  <xsl:apply-templates select="." />
                </xsl:otherwise>
              </xsl:choose>
            </xsl:for-each>
          </NodeList>
        </NodeBody>
      </Node>
      <!--  Insert return value command if not present -->
      <xsl:if
        test="not(.//Command/Name/StringValue/text() = 'SendReturnValue')">
        <Node NodeType="Command" epx="aux">
          <NodeId generated="1">
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


  <xsl:template name="run-wait-command">
    <xsl:param name="command" />
    <xsl:param name="dest" />
    <xsl:param name="args" />
    <Node NodeType="Command" epx="aux">
      <NodeId generated="1">
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

  <xsl:template name="on-command-command">
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

  <!-- Functions -->

  <!-- Return the (total) number of seconds in and ISO 8601 duration -->
  <xsl:function name="tr:seconds">
    <xsl:param name="duration"/>
    <xsl:value-of select="xs:dayTimeDuration($duration) div xs:dayTimeDuration('PT1.0S')"/>
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
