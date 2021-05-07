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

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

  <!-- If-Then-Elseif-Else -->

  <!-- TODO: Put attributes in sorted order -->

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
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
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
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
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
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
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
          <xsl:call-template name="copy-source-locator-attributes">
            <xsl:with-param name="context" select="$expanded-clause/Node" />
          </xsl:call-template>
          <xsl:call-template name="default-node-id">
            <xsl:with-param name="context" select="$expanded-clause/Node" />
            <xsl:with-param name="name" select="name(.)" />
          </xsl:call-template>
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

</xsl:transform>
