<?xml version="1.0" encoding="ISO-8859-1"?>

<!--
* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

  <!-- SynchronousCommand -->

  <xsl:template match="SynchronousCommand">

    <xsl:variable name="expandedCommand">
      <xsl:apply-templates select="Command" />
    </xsl:variable>

    <!-- Is the command return value used? -->
    <xsl:variable name="assignmentVariable">
      <xsl:call-template name="assignment-variable">
        <xsl:with-param name="context" select="$expandedCommand/Command" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$assignmentVariable/*">
        <xsl:call-template name="sync-cmd-with-return">
          <xsl:with-param name="assignmentVariable"
                          select="$assignmentVariable/*" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="sync-cmd-base" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- 
       If the Command invocation assigns the return value, the Command
       node's end condition is that the return value is known.  Since
       the result variable's value may already be known at entry, we
       have to declare a temporary variable to receive the return
       value.  This means wrapping a list node around the Command
       element.

       In the case of an ArrayVariable or ArrayElement, we have to
       find the declaration of the array to identify the element type,
       both for the temporary variable declaration, and for
       ArrayElement, the RHS element of the Assignment node as well.
  -->

  <xsl:template name="sync-cmd-with-return">
    <xsl:param name="assignmentVariable" required="yes" />

    <xsl:variable name="tempVarName"
                  select="tr:prefix('SynchronousCommand_temp')" />

    <xsl:choose>
      <xsl:when test="$assignmentVariable[name()='ArrayElement']">
        <xsl:call-template name="sync-cmd-with-array-elt-var">
          <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
          <xsl:with-param name="tempVarName" select="$tempVarName" />
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="$assignmentVariable[name()='ArrayVariable']">
        <xsl:call-template name="sync-cmd-with-array-result-var">
          <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
          <xsl:with-param name="tempVarName" select="$tempVarName" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="sync-cmd-with-scalar-result-var">
          <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
          <xsl:with-param name="tempVarName" select="$tempVarName" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="sync-cmd-with-scalar-result-var">
    <xsl:param name="assignmentVariable" required="yes" />
    <xsl:param name="tempVarName" required="yes" />

    <xsl:variable name="tempVarType">
      <xsl:value-of select="fn:substring-before(name($assignmentVariable),'Variable')" />
    </xsl:variable>

    <xsl:call-template name="sync-cmd-return-common">
      <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
      <xsl:with-param name="tempVarType" select="$tempVarType" />
      <xsl:with-param name="tempVarRef">
        <xsl:element name="{$assignmentVariable/name()}">
          <xsl:value-of select="$tempVarName" />
        </xsl:element>
      </xsl:with-param>
      <xsl:with-param name="tempVarDecl">
        <DeclareVariable>
          <Name><xsl:value-of select="$tempVarName" /></Name>
          <Type><xsl:value-of select="$tempVarType" /></Type>
        </DeclareVariable>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="sync-cmd-with-array-result-var" >
    <xsl:param name="assignmentVariable" required="yes" />
    <xsl:param name="tempVarName" required="yes" />

    <xsl:variable name="arrayDecl">
      <xsl:call-template name="array-declaration">
        <xsl:with-param name="array-name"
                        select="$assignmentVariable/text()" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:call-template name="sync-cmd-return-common">
      <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
      <xsl:with-param name="tempVarType">Array</xsl:with-param>
      <xsl:with-param name="tempVarRef">
        <xsl:element name="{name($assignmentVariable)}">
          <xsl:value-of select="$tempVarName" />
        </xsl:element>
      </xsl:with-param>
      <xsl:with-param name="tempVarDecl">
        <DeclareArray>
          <Name><xsl:value-of select="$tempVarName" /></Name>
          <xsl:copy-of select="$arrayDecl/DeclareArray/Type" />
          <xsl:copy-of select="$arrayDecl/DeclareArray/MaxSize" />
        </DeclareArray>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="sync-cmd-with-array-elt-var">
    <xsl:param name="assignmentVariable" required="yes" />
    <xsl:param name="tempVarName" required="yes" />

    <!-- N.B. Anything other than a <Name> or <ArrayVariable> is doomed to fail. -->
    <xsl:variable name="arrayDecl">
      <xsl:call-template name="array-declaration">
        <xsl:with-param name="array-name"
                        select="$assignmentVariable/(Name|ArrayVariable)/text()" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="tempVarType"
                  select="$arrayDecl/Type/text()" />

    <xsl:call-template name="sync-cmd-return-common">
      <xsl:with-param name="assignmentVariable" select="$assignmentVariable" />
      <xsl:with-param name="tempVarType" select="$tempVarType" />
      <xsl:with-param name="tempVarRef">
        <xsl:element name="{concat($tempVarType, 'Variable')}">
          <xsl:value-of select="$tempVarName" />
        </xsl:element>
      </xsl:with-param>
      <xsl:with-param name="tempVarDecl">
        <DeclareVariable>
          <Name><xsl:value-of select="$tempVarName" /></Name>
          <xsl:copy-of select="$arrayDecl/Type" />
        </DeclareVariable>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <!-- Common components of above -->
  <xsl:template name="sync-cmd-return-common">
    <xsl:param name="assignmentVariable" required="yes" />
    <xsl:param name="tempVarDecl" required="yes" />
    <xsl:param name="tempVarRef" required="yes" />
    <xsl:param name="tempVarType" required="yes" />

    <xsl:variable name="commandNodeId">
      <xsl:value-of select="tr:prefix('SynchronousCommand_cmd')" />
    </xsl:variable>
    <xsl:variable name="assignNodeId">
      <xsl:value-of select="tr:prefix('SynchronousCommand_assign')"/>
    </xsl:variable>

    <xsl:variable name="userConditions">
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>

    <xsl:variable name="resultRHS">
      <xsl:choose>
        <xsl:when test="$tempVarType = 'Array'
                        or $tempVarType = 'Boolean'
                        or $tempVarType = 'String'">
          <xsl:value-of select="concat($tempVarType, 'RHS')" />
        </xsl:when>
        <xsl:otherwise>NumericRHS</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <Node NodeType="NodeList" epx="SynchronousCommand_wrapper">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <xsl:apply-templates select="UsingMutex" />
      <VariableDeclarations>
        <xsl:apply-templates select="VariableDeclarations/*" />
        <xsl:sequence select="$tempVarDecl" />
      </VariableDeclarations>

      <xsl:sequence select="$userConditions/(ExitCondition|PreCondition|
                            RepeatCondition|SkipCondition|StartCondition)" />
      <xsl:call-template name="sync-cmd-wrapper-invariant">
        <xsl:with-param name="commandNodeId" select="$commandNodeId" />
        <xsl:with-param name="userInvariant" select="$userConditions/InvariantCondition" />
      </xsl:call-template>
      <xsl:call-template name="sync-cmd-wrapper-end">
        <xsl:with-param name="commandNodeId" select="$commandNodeId" />
        <xsl:with-param name="assignNodeId" select="$assignNodeId" />
        <xsl:with-param name="userEnd" select="$userConditions/EndCondition" />
      </xsl:call-template>
      <xsl:call-template name="sync-cmd-wrapper-post">
        <xsl:with-param name="commandNodeId" select="$commandNodeId" />
        <xsl:with-param name="assignNodeId" select="$assignNodeId" />
        <xsl:with-param name="userPost" select="$userConditions/PostCondition" />
      </xsl:call-template>

      <NodeBody>
        <NodeList>
          <Node NodeType="Command" epx="SynchronousCommandCommand">
            <NodeId><xsl:value-of select="$commandNodeId" /></NodeId>
            <xsl:if test="Timeout">
              <InvariantCondition>
                <xsl:call-template name="sync-cmd-timeout-invariant" />
              </InvariantCondition>
            </xsl:if>
            <EndCondition>
              <IsKnown><xsl:copy-of select="$tempVarRef" /></IsKnown>
            </EndCondition>
            <xsl:call-template name="sync-cmd-post" />
            <NodeBody>
              <Command>
                <xsl:apply-templates select="Command/ResourceList" />                  
                <xsl:copy-of select="$tempVarRef" />
                <xsl:apply-templates select="Command/(Name|Arguments)" />
              </Command>
            </NodeBody>
          </Node>
          <Node NodeType="Assignment" epx="SynchronousCommandAssignment">
            <NodeId><xsl:value-of select="$assignNodeId" /></NodeId>
            <StartCondition>
              <xsl:choose>
                <xsl:when test="Checked">
                  <AND>
                    <EQInternal>
                      <xsl:call-template name="command-handle-ref">
                        <xsl:with-param name="dir">sibling</xsl:with-param>
                        <xsl:with-param name="nodeId" select="$commandNodeId" />
                      </xsl:call-template>
                      <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                    </EQInternal>
                    <IsKnown><xsl:copy-of select="$tempVarRef" /></IsKnown>
                  </AND>
                </xsl:when>
                <xsl:otherwise>
                  <IsKnown><xsl:copy-of select="$tempVarRef" /></IsKnown>
                </xsl:otherwise>
              </xsl:choose>
            </StartCondition>
            <NodeBody>
              <Assignment>
                <xsl:copy-of select="$assignmentVariable" />
                <xsl:element name="{$resultRHS}">
                  <xsl:copy-of select="$tempVarRef" />
                </xsl:element>
              </Assignment>
            </NodeBody>
          </Node>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="sync-cmd-wrapper-invariant">
    <xsl:param name="commandNodeId" required="yes" />
    <xsl:param name="userInvariant" />

    <!-- boilerplate -->
    <xsl:variable name="commandHandleRef">
      <xsl:call-template name="command-handle-ref">
        <xsl:with-param name="dir">child</xsl:with-param>
        <xsl:with-param name="nodeId" select="$commandNodeId" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="Checked">
        <InvariantCondition>
          <AND>
            <xsl:sequence select="$userInvariant/*"/>
            <NEInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_DENIED</NodeCommandHandleValue>
            </NEInternal>
            <NEInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_FAILED</NodeCommandHandleValue>
            </NEInternal>
            <NEInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_INTERFACE_ERROR</NodeCommandHandleValue>
            </NEInternal>
            <NEInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_ABORTED</NodeCommandHandleValue>
            </NEInternal>
            <NEInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_ABORT_FAILED</NodeCommandHandleValue>
            </NEInternal>
          </AND>
        </InvariantCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$userInvariant"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="sync-cmd-wrapper-end">
    <xsl:param name="commandNodeId" required="yes" />
    <xsl:param name="assignNodeId" required="yes" />
    <xsl:param name="userEnd" />

    <xsl:choose>
      <xsl:when test="Checked">
        <EndCondition>
          <OR>
            <xsl:sequence select="$userEnd/*" />
            <Failed>
              <NodeRef dir="$child"><xsl:value-of select="$commandNodeId" /></NodeRef>
            </Failed>
            <Succeeded>
              <NodeRef dir="$child"><xsl:value-of select="$assignNodeId" /></NodeRef>
            </Succeeded>
          </OR>
        </EndCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$userEnd" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="sync-cmd-wrapper-post">
    <xsl:param name="commandNodeId" required="yes" />
    <xsl:param name="assignNodeId" required="yes" />
    <xsl:param name="userPost" />

    <!-- boilerplate -->
    <xsl:variable name="commandHandleRef">
      <xsl:call-template name="command-handle-ref">
        <xsl:with-param name="dir">child</xsl:with-param>
        <xsl:with-param name="nodeId" select="$commandNodeId" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="Checked">
        <PostCondition>
          <AND>
            <xsl:sequence select="$userPost/*" />
            <EQInternal>
              <xsl:copy-of select="$commandHandleRef" />
              <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
            </EQInternal>
            <Succeeded>
              <NodeRef dir="child"><xsl:value-of select="$assignNodeId" /></NodeRef>
            </Succeeded>
          </AND>
        </PostCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$userPost" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- 
       Synchronous command without an assignment simply expands into a
       Command node with condition wrappers
  -->
  <xsl:template name="sync-cmd-base">
    <xsl:variable name="userConditions">
      <xsl:apply-templates select="tr:conditions(.)" />
    </xsl:variable>

    <Node NodeType="Command" epx="SynchronousCommand">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <xsl:apply-templates select="(VariableDeclarations|UsingMutex)" />

      <xsl:sequence select="$userConditions/(ExitCondition|PreCondition|
                            RepeatCondition|SkipCondition|StartCondition)" />
      <xsl:call-template name="sync-cmd-base-invariant">
        <xsl:with-param name="userInvariant" select="$userConditions/InvariantCondition" />
      </xsl:call-template>
      <xsl:call-template name="sync-cmd-base-end">
        <xsl:with-param name="userEnd" select="$userConditions/EndCondition" />
      </xsl:call-template>
      <xsl:call-template name="sync-cmd-post">
        <xsl:with-param name="userPost" select="$userConditions/PostCondition" />
      </xsl:call-template>
      <NodeBody>
        <xsl:apply-templates select="Command"/>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="sync-cmd-base-invariant">
    <xsl:param name="userInvariant" />
    <xsl:choose>
      <xsl:when test="Timeout and $userInvariant">
        <InvariantCondition>
          <AND>
            <xsl:sequence select="$userInvariant/*" />
            <xsl:call-template name="sync-cmd-timeout-invariant" />
          </AND>
        </InvariantCondition>
      </xsl:when>
      <xsl:when test="Timeout">
        <InvariantCondition>
          <xsl:call-template name="sync-cmd-timeout-invariant" />
        </InvariantCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$userInvariant" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="sync-cmd-base-end">
    <xsl:param name="userEnd" />

    <xsl:variable name="myCommandHandleRef">
      <xsl:call-template name="command-handle-ref" />
    </xsl:variable>

    <EndCondition>
      <OR>
        <xsl:sequence select="$userEnd/*" />
        <EQInternal>
          <xsl:copy-of select="$myCommandHandleRef" />
          <NodeCommandHandleValue>COMMAND_DENIED</NodeCommandHandleValue>
        </EQInternal>
        <EQInternal>
          <xsl:copy-of select="$myCommandHandleRef" />
          <NodeCommandHandleValue>COMMAND_INTERFACE_ERROR</NodeCommandHandleValue>
        </EQInternal>
        <EQInternal>
          <xsl:copy-of select="$myCommandHandleRef" />
          <NodeCommandHandleValue>COMMAND_FAILED</NodeCommandHandleValue>
        </EQInternal>
        <EQInternal>
          <xsl:copy-of select="$myCommandHandleRef" />
          <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
        </EQInternal>
      </OR>
    </EndCondition>
  </xsl:template>

  <xsl:template name="sync-cmd-post">
    <xsl:param name="userPost" />
    <xsl:variable name="checkPostcondition">
      <EQInternal>
        <xsl:call-template name="command-handle-ref" />
        <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
      </EQInternal>
    </xsl:variable>
    <xsl:choose>
      <xsl:when test="Checked and $userPost">
        <PostCondition>
          <AND>
            <xsl:sequence select="$userPost/*" />
            <xsl:copy-of select="$checkPostcondition" />
          </AND>
        </PostCondition>
      </xsl:when>
      <xsl:when test="Checked">
        <PostCondition>
          <xsl:copy-of select="$checkPostcondition" />
        </PostCondition>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$userPost" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Timeout test for invariant condition --> 
  <xsl:template name="sync-cmd-timeout-invariant">
    <LT>
      <LookupOnChange>
        <Name>
          <StringValue>time</StringValue>
        </Name>
        <xsl:apply-templates select="Timeout/Tolerance" />
      </LookupOnChange>
      <ADD>
        <xsl:apply-templates select="Timeout/*[1]" />
        <NodeTimepointValue>
          <NodeRef dir="self" />
          <NodeStateValue>EXECUTING</NodeStateValue>
          <Timepoint>START</Timepoint>
        </NodeTimepointValue>
      </ADD>
    </LT>
  </xsl:template>

</xsl:transform>
