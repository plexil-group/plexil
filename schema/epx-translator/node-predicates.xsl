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

</xsl:transform>
