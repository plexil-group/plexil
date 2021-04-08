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

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

  <!-- While loop -->

  <xsl:template match="While">
    <Node NodeType="NodeList" epx="While">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="standard-preamble" />
      <xsl:call-template name="while-body" />
    </Node>
  </xsl:template>

  <xsl:template name="while-body"> 
    <xsl:choose>
      <xsl:when test="VariableDeclarations|UsingMutex|RepeatCondition|StartCondition">
        <!-- must create outer wrapper node -->
        <NodeBody>
          <NodeList>
            <Node NodeType="NodeList" epx="While_wrapper">
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
        <!-- Note: context is still the While element -->
        <xsl:call-template name="while-body-2">
          <xsl:with-param name="test-id" select="$test-id"/>
        </xsl:call-template>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template name="while-body-2">
    <xsl:param name="test-id" required="yes" />
    <xsl:variable name="expanded-action">
      <xsl:apply-templates select="fn:exactly-one(Action/*)" />
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
          <xsl:call-template name="copy-source-locator-attributes">
            <xsl:with-param name="context" select="$expanded-action/Node" />
          </xsl:call-template>
          <xsl:call-template name="default-node-id">
            <xsl:with-param name="context" select="$expanded-action/Node" />
            <xsl:with-param name="name" select="name(.)" />
          </xsl:call-template>
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

</xsl:transform>
