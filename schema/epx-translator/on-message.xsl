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

<!-- This stylesheet requires XSLT 2.0 for xsl:sequence -->

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

  <!-- OnMessage macro -->

  <xsl:template match="OnMessage">
    <xsl:variable name="Msg_staging">
      <xsl:call-template name="OnMessage-staging" />
    </xsl:variable>
    <xsl:apply-templates select="$Msg_staging" />
  </xsl:template>

  <xsl:template name="OnMessage-staging">
    <xsl:variable name="maybe-anonymous-body" as="element()?" >
      <xsl:sequence select="fn:zero-or-one(tr:actions(.))" />
    </xsl:variable>
    <!-- Ensure body, if it exists, has a NodeId; gensym one if not -->
    <xsl:variable name="maybe-body">
      <xsl:choose>
        <xsl:when test="$maybe-anonymous-body/NodeId">
          <xsl:sequence select="$maybe-anonymous-body" />
        </xsl:when>
        <xsl:when test="$maybe-anonymous-body">
          <!-- Add a generated NodeId -->
          <Node>
            <xsl:copy-of select="$maybe-anonymous-body/@*" />
            <NodeId generated="1">
              <xsl:copy-of select="tr:prefix('OnMessage_body')" />
            </NodeId>
            <xsl:copy-of select="$maybe-anonymous-body/*" />
          </Node>
        </xsl:when>
      </xsl:choose>
    </xsl:variable>
    <CheckedSequence>
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <VariableDeclarations>
        <DeclareVariable>
          <Name>
            <xsl:value-of select="tr:prefix('hdl')" />
          </Name>
          <Type>String</Type>
        </DeclareVariable>
      </VariableDeclarations>
      <xsl:sequence select="UsingMutex" />

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

      <!-- Expand body (may be empty) -->
      <xsl:apply-templates select="$maybe-body" />
    </CheckedSequence>
  </xsl:template>

  <!-- Construct the receive wait Command node for OnMessage -->
  <xsl:template name="run-wait-command">
    <xsl:param name="command" />
    <xsl:param name="dest" />
    <xsl:param name="args" />
    <Node NodeType="Command" epx="aux">
      <NodeId generated="1">
        <xsl:copy-of select="tr:prefix('OnMessage_MsgWait')" />
      </NodeId>
      <EndCondition>
        <IsKnown>
          <xsl:sequence select="$dest" />
        </IsKnown>
      </EndCondition>
      <!-- If the command fails (e.g. due to interface error), it can terminate early
           with node outcome SUCCEEDED. -->
      <PostCondition>
        <EQInternal>
          <NodeCommandHandleVariable><NodeRef dir="self"/></NodeCommandHandleVariable>
          <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
        </EQInternal>
      </PostCondition>
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

</xsl:transform>
