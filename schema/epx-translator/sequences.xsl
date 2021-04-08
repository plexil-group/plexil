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

  <!-- Sequence, CheckedSequence, UncheckedSequence -->
  <!-- We may switch Sequence to be an UncheckedSequence
       in future releases -->

  <xsl:template match="UncheckedSequence">
    <xsl:call-template name="UncheckedSequence" />
  </xsl:template>

  <xsl:template name="UncheckedSequence">
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="standard-preamble" />
      <xsl:call-template name="sequence-body" />
    </Node>
  </xsl:template>

  <xsl:template match="Sequence|CheckedSequence">
    <xsl:call-template name="CheckedSequence" />
  </xsl:template>

  <xsl:template name="CheckedSequence">
    <xsl:variable name="conds">
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>
    
    <Node NodeType="NodeList" epx="{name(.)}">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <xsl:apply-templates select="VariableDeclarations|UsingMutex" />
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
        <xsl:apply-templates select="tr:actions(.)" />
      </xsl:with-param>
    </xsl:call-template>
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
                            VariableDeclarations|UsingMutex|Assume|Desire|Expect" />
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

</xsl:transform>
