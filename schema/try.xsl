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

  <!-- Try (not to be confused with try/catch) -->

  <xsl:template match="Try">
    <xsl:variable name="conds">
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>
    <xsl:variable name="expanded-children">
      <xsl:apply-templates select="tr:actions(.)" />
    </xsl:variable>

    <Node NodeType="NodeList" epx="Try">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <xsl:apply-templates select="VariableDeclarations|UsingMutex" />
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

</xsl:transform>
