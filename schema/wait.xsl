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

  <!-- Wait utility macro -->

  <xsl:template match="Wait">
    <xsl:variable name="conds">
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>

    <Node NodeType="Empty" epx="Wait">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <xsl:apply-templates select="VariableDeclarations|UsingMutex" />
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

</xsl:transform>
