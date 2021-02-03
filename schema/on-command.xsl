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

  <!-- OnCommand

This is a macro designed to work with the IpcAdapter commands ReceiveCommand,
GetParameter, and SendReturnValue. It looks like:

<OnCommand>
<VariableDeclarations> ... command parameters... </VariableDeclarations>
<Name><StringValue>name-of-command</StringValue></Name>

action

</OnCommand>

  -->

  <xsl:template match="OnCommand">
    <xsl:variable name="Cmd_staging">
      <xsl:call-template name="OnCommand-staging" />
    </xsl:variable>
    <xsl:apply-templates select="$Cmd_staging" />
  </xsl:template>

  <xsl:template name="OnCommand-staging">
    <!-- Name of the command identifier ("handle") variable -->
    <xsl:variable name="hdl_name">
      <xsl:value-of select="tr:prefix('hdl')" />
    </xsl:variable>

    <Node NodeType="NodeList" epx="OnCommand">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="handle-common-clauses" />
      <VariableDeclarations>
        <!-- These are the command parameters -->
        <xsl:apply-templates select="VariableDeclarations/(DeclareVariable|DeclareArray)" />
        <!-- Declare the command identifier variable -->
        <DeclareVariable>
          <Name><xsl:value-of select="$hdl_name" /></Name>
          <Type>String</Type>
        </DeclareVariable>
        <!-- Mutexes are variables too (but not command parameters) -->
        <xsl:apply-templates select="VariableDeclarations/DeclareMutex" />
      </VariableDeclarations>
      <xsl:apply-templates select="UsingMutex" />

      <!-- Handle the OnCommand node conditions -->

      <xsl:variable name="invariant">
          <NoChildFailed>
            <NodeRef dir="self"/>
          </NoChildFailed>
      </xsl:variable>
      <xsl:choose>
        <xsl:when test="InvariantCondition" >
          <!-- Combine ours with theirs -->
          <xsl:apply-templates select="tr:conditions(.) except InvariantCondition" />
          <InvariantCondition>
            <AND>
              <xsl:apply-templates select="InvariantCondition/*"/>
              <xsl:sequence select="$invariant"/>
            </AND>
          </InvariantCondition>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="tr:conditions(.)" />
          <InvariantCondition>
            <xsl:sequence select="$invariant"/>
          </InvariantCondition>
        </xsl:otherwise>
      </xsl:choose>

      <!-- The command name for which we wait -->
      <xsl:variable name="cmdName">
        <xsl:value-of select="Name/StringValue" />
      </xsl:variable>
      <!-- Command identifier variable, as a variable reference -->
      <xsl:variable name="hdl_ref">
        <StringVariable><xsl:value-of select="$hdl_name" /></StringVariable>
      </xsl:variable>

      <NodeBody>
        <NodeList>
          <!-- Generate command wait command node -->
          <xsl:call-template name="on-command-command-wait">
            <xsl:with-param name="cmdName" select="$cmdName" />
            <xsl:with-param name="dest" select="$hdl_ref" />
          </xsl:call-template>
      
          <!-- Generate get parameters command nodes -->
          <xsl:for-each select="VariableDeclarations/(DeclareVariable|DeclareArray)">
            <xsl:call-template name="get-param-command-node">
              <xsl:with-param name="hdl_ref" select="$hdl_ref"/>
              <xsl:with-param name="cmdName" select="$cmdName" />
              <xsl:with-param name="index" select="position() - 1"/>
            </xsl:call-template>
          </xsl:for-each>

          <xsl:variable name="body-expansion" as="element(Node)">
            <xsl:apply-templates select="fn:exactly-one(tr:actions(.))" />
          </xsl:variable>

          <xsl:variable name="action-id-elt" as="element(NodeId)" >
            <xsl:call-template name="default-node-id">
              <xsl:with-param name="context" select="$body-expansion"/>
              <xsl:with-param name="name">OnCommand_action</xsl:with-param>
            </xsl:call-template>
          </xsl:variable>

          <xsl:call-template name="on-command-action-body" >
            <xsl:with-param name="action" select="$body-expansion" />
            <xsl:with-param name="action-id-elt" select="$action-id-elt" />
            <xsl:with-param name="hdl_ref" select="$hdl_ref"/>
            <xsl:with-param name="nParams" as="xs:integer" 
                            select="fn:count(VariableDeclarations/(DeclareVariable|DeclareArray))" />
          </xsl:call-template>
          
          <!-- Insert return value command if not present in expansion -->
          <xsl:if test="not(.//Command/Name/StringValue/text() = 'SendReturnValue')">
            <Node NodeType="Command" epx="OnCommand-return">
              <NodeId generated="1">
                <xsl:value-of select="fn:concat(tr:prefix('OnCommandReturn'), '_', $cmdName)" />
              </NodeId>
              <StartCondition>
                <Finished>
                  <NodeRef dir="sibling">
                    <xsl:value-of select="$action-id-elt/text()" />
                  </NodeRef>
                </Finished>
              </StartCondition>
              <NodeBody>
                <Command>
                  <Name>
                    <StringValue>SendReturnValue</StringValue>
                  </Name>
                  <Arguments>
                    <xsl:sequence select="$hdl_ref"/>
                    <BooleanValue>true</BooleanValue>
                  </Arguments>
                </Command>
              </NodeBody>
            </Node>
          </xsl:if>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- Construct the receive wait node for OnMessage and OnCommand -->
  <xsl:template name="on-command-command-wait">
    <xsl:param name="dest" />     <!-- Variable reference for the command "handle". -->
    <xsl:param name="cmdName" />  <!-- The command name for which we wait. -->
    <Node NodeType="Command" epx="OnCommand-command-wait">
      <NodeId generated="1">
        <xsl:copy-of select="tr:prefix('CmdWait')" />   <!-- FIXME add comand name -->
      </NodeId>
      <EndCondition>
        <IsKnown> <xsl:sequence select="$dest" /> </IsKnown>
      </EndCondition>
      <NodeBody>
        <Command>
          <xsl:sequence select="$dest" />
          <Name><StringValue>ReceiveCommand</StringValue></Name>
          <Arguments>
            <StringValue><xsl:value-of select="$cmdName" /></StringValue>
          </Arguments>
        </Command>
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- Construct one GetParameter command node for OnCommand -->
  <!-- . is one of the DeclareVariable or DeclareArray elements -->
  <xsl:template name="get-param-command-node">
    <xsl:param name="hdl_ref"/>     <!-- Reference to the "handle" variable -->
    <xsl:param name="cmdName" />    <!-- Name of the command we're getting parms for -->
    <xsl:param name="index"/>       <!-- position in the declaration list, 0-based -->

    <Node NodeType="Command" epx="OnCommand-get-param">
      <NodeId generated="1">
        <xsl:call-template name="param-node-id">
          <xsl:with-param name="index" select="$index" />
        </xsl:call-template>
      </NodeId>
      <StartCondition>
        <IsKnown><xsl:sequence select="$hdl_ref" /></IsKnown>
      </StartCondition>
      <EndCondition>
        <IsKnown>
          <xsl:choose>
            <xsl:when test="self::DeclareArray"> <!-- Arrays -->
              <ArrayElement>
                <Name><xsl:value-of select="Name"/></Name>
                <Index><IntegerValue>0</IntegerValue></Index>
              </ArrayElement>
            </xsl:when>
            <xsl:when test="self::DeclareVariable"> <!-- Scalars -->
              <xsl:element name='{concat(Type/text(), "Variable")}'>
                <xsl:value-of select="Name/text()" />
              </xsl:element>
            </xsl:when>
            <xsl:otherwise> <!-- Something else -->
              <errorContext><xsl:sequence select="." /></errorContext>
            </xsl:otherwise>
          </xsl:choose>
        </IsKnown>
      </EndCondition>
      <NodeBody>
        <Command>
          <xsl:choose>
            <xsl:when test="self::DeclareArray"> <!-- Arrays -->
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
            <xsl:copy-of select="$hdl_ref" />
            <IntegerValue><xsl:value-of select="$index" /></IntegerValue>
          </Arguments>
        </Command>
      </NodeBody>
    </Node>
  </xsl:template>

  <!-- Construct the action body -->
  <xsl:template name="on-command-action-body">
    <xsl:param name="action" />
    <xsl:param name="action-id-elt"/>
    <xsl:param name="hdl_ref"/>     <!-- Reference to the "handle" variable -->
    <xsl:param name="nParams" as="xs:integer" />    <!-- Number of parameters to be gotten -->

    <xsl:variable name="body" as="element(NodeBody)?">
      <xsl:copy-of select="$action/NodeBody" />
    </xsl:variable>

    <xsl:variable name="amendedBody" >
      <xsl:call-template name="amend-send-return" >
        <xsl:with-param name="ctxt" select="$body" />
        <xsl:with-param name="hdl_ref" select="$hdl_ref" />
      </xsl:call-template>
    </xsl:variable>

    <Node epx="OnCommand-action">
      <xsl:copy-of select="$action/@NodeType"/>
      <xsl:call-template name="copy-source-locator-attributes">
        <xsl:with-param name="context" select="$action"/>
      </xsl:call-template> 
      <xsl:copy-of select="$action-id-elt" />
      <xsl:call-template name="handle-common-clauses">
        <xsl:with-param name="context" select="$action"/>
      </xsl:call-template>
      <xsl:copy-of select="$action/(VariableDeclarations|UsingMutex)" />
      <xsl:copy-of select="tr:conditions($action) except StartCondition" />
      <StartCondition>
        <xsl:call-template name="on-command-body-start-condition" >
          <xsl:with-param name="hdl_ref" select="$hdl_ref"/>
          <xsl:with-param name="nParams" select="$nParams" />
        </xsl:call-template>
      </StartCondition>
      <xsl:if test="$action/NodeBody">
        <NodeBody>
          <xsl:copy-of select="$amendedBody"/>
        </NodeBody>
      </xsl:if>
    </Node>

  </xsl:template>

  <!-- Construct the start condition expression for the body -->
  <xsl:template name="on-command-body-start-condition">
    <xsl:param name="hdl_ref"/>     <!-- Reference to the "handle" variable -->
    <xsl:param name="nParams" as="xs:integer" />    <!-- Number of parameters to be gotten -->
    <xsl:choose>
      <!-- No params -->
      <xsl:when test="$nParams = 0">
        <IsKnown><xsl:sequence select="$hdl_ref" /></IsKnown>
      </xsl:when>
      <!-- One param -->
      <xsl:when test="$nParams = 1">
        <Succeeded>
          <NodeRef dir="sibling">
            <xsl:call-template name="param-node-id">
              <xsl:with-param name="index" select="0" />
            </xsl:call-template>
          </NodeRef>
        </Succeeded>
      </xsl:when>
      <!-- General case -->
      <xsl:otherwise>
        <AND>
          <xsl:for-each select="0 to ($nParams - 1)">
            <Succeeded>
              <NodeRef dir="sibling">
                <xsl:call-template name="param-node-id">
                  <xsl:with-param name="index" select="." />
                </xsl:call-template>
              </NodeRef>
            </Succeeded>
          </xsl:for-each>
        </AND>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- The NodeId of the nth parameter grabbing node. -->
  <xsl:template name="param-node-id">
    <xsl:param name="index"/>       <!-- position in the declaration list, 0-based -->
    <xsl:value-of select="concat(tr:prefix('CmdGetParam'), '_', $index)" />
  </xsl:template>

  <!-- Recurse looking for a SendReturnValue command -->
  <!-- Called on NodeBody which has already been expanded -->
  <xsl:template name="amend-send-return">
    <xsl:param name="ctxt" />
    <xsl:param name="hdl_ref" />

    <xsl:choose>
      <xsl:when test="$ctxt/NodeList">
        <NodeList>
          <xsl:for-each select="$ctxt/NodeList/*">
            <!-- Context here should be a Node -->
            <Node>
              <xsl:copy-of select="@*" />
              <xsl:copy-of select="* except NodeBody" />
              <xsl:if test="NodeBody" >
                <NodeBody>
                  <xsl:call-template name="amend-send-return">
                    <xsl:with-param name="ctxt" select="NodeBody" />
                    <xsl:with-param name="hdl_ref" select="$hdl_ref" />
                  </xsl:call-template>
                </NodeBody>
              </xsl:if>
            </Node>
          </xsl:for-each>
        </NodeList>
      </xsl:when>
      <xsl:when test="$ctxt/Command/Name/StringValue/text()='SendReturnValue'">
        <Command>
          <xsl:sequence select="NodeBody/Command/(* except Arguments)" />
          <Arguments>
            <xsl:sequence select="$hdl_ref" />
            <xsl:sequence select="NodeBody/Command/Arguments/*" />
          </Arguments>
        </Command>
      </xsl:when>
      <xsl:otherwise>
        <xsl:sequence select="$ctxt/*" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:transform>
