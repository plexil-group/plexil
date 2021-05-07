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

  <!-- Do-while loop -->

  <!-- FIXME: Identify generated node IDs -->
  <!-- FIXME: how to handle repeat condition suppled to Do? -->
  <xsl:template match="Do">
    <xsl:variable name="expanded-conditions" as="element()*" >
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>
    <xsl:variable name="expanded-declarations" as="element()*">
      <xsl:apply-templates select="VariableDeclarations|UsingMutex" />
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="$expanded-declarations or $expanded-conditions or Action/Interface">
        <!-- must create outer wrapper node -->
        <Node NodeType="NodeList" epx="DoWrapper">
          <xsl:call-template name="copy-source-locator-attributes" />
          <xsl:call-template name="ensure-unique-node-id" />
          <xsl:call-template name="handle-common-clauses" />
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
        <!-- Straightforward case, just add a RepeatCondition -->
        <!-- FIXME: no place to hang a unique NodeId -->
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
          <!-- FIXME: possible collision between original and child of expansion? -->
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

</xsl:transform>
