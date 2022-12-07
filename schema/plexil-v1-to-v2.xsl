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


<xsl:stylesheet version="2.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xs="http://www.w3.org/2001/XMLSchema"
                xmlns:fn="http://www.w3.org/2005/xpath-functions"
                xmlns:tr="translate-plexil.xsl"
                exclude-result-prefixes="fn tr xs xsl"
                >

  <xsl:output method="xml" indent="yes"/> <!-- indent="no" when debugged -->

  <xsl:preserve-space elements="StringValue" />
  <xsl:strip-space elements="*" />

  <!-- Selector keys -->

  <xsl:key name="Conditions"
           match="EndCondition|ExitCondition|InvariantCondition|PostCondition|
                  PreCondition|RepeatCondition|SkipCondition|StartCondition"
           use="." />

  <xsl:key name="DeclaredVariable"
           match="ArrayElement|ArrayVariable|BooleanVariable|
                  IntegerVariable|RealVariable|StringVariable"
           use="." />

  <xsl:key name="LiteralValue"
           match="BooleanValue|IntegerValue|RealValue|StringValue"
           use="." />

  <!-- This is the "overriding copy idiom", from "XSLT Cookbook" by
       Sal Mangano.  It is the identity transform, covering all
       elements that are not explicitly handled elsewhere. -->

  <xsl:template match="@* | node()">
    <xsl:copy
        copy-namespaces="no"> <!-- XSLT 2.0 feature -->
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

  <!-- Entry point -->
  <xsl:template match="PlexilPlan">
    <PlexilPlan version="2.0">
      <xsl:copy-of select="@FileName" />
      <!-- 0 or 1 expected -->
      <xsl:apply-templates select="GlobalDeclarations" />
      <!-- 1 expected -->
      <xsl:apply-templates select="Node" />
    </PlexilPlan>
  </xsl:template>

  <xsl:template match="GlobalDeclarations">
    <GlobalDeclarations>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:apply-templates />
    </GlobalDeclarations>
  </xsl:template>

  <xsl:template match="CommandDeclaration">
    <CommandDeclaration>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:attribute name="Name">
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:if test="Return">
        <xsl:attribute name="Type">
          <xsl:value-of select="Return/Type" />
          <xsl:if test="Return/MaxSize">
            <xsl:text>Array</xsl:text>
          </xsl:if>
        </xsl:attribute>
        <xsl:if test="Return/MaxSize">
          <xsl:attribute name="MaxSize">
            <xsl:value-of select="Return/MaxSize" />
          </xsl:attribute>
        </xsl:if>
      </xsl:if>
      <xsl:apply-templates select="Parameter" />
      <xsl:apply-templates select="AnyParameters" />
      <xsl:apply-templates select="ResourceList" />
    </CommandDeclaration>
  </xsl:template>

  <xsl:template match="StateDeclaration">
    <StateDeclaration>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:attribute name="Name">
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:attribute name="Type">
        <xsl:value-of select="Return/Type" />
        <xsl:if test="Return/MaxSize">
          <xsl:text>Array</xsl:text>
        </xsl:if>
      </xsl:attribute>
      <xsl:if test="Return/MaxSize">
        <xsl:attribute name="MaxSize">
          <xsl:value-of select="Return/MaxSize" />
        </xsl:attribute>
      </xsl:if>
      <xsl:apply-templates select="Parameter"/>
      <xsl:apply-templates select="AnyParameters" />
    </StateDeclaration>
  </xsl:template>

  <xsl:template match="LibraryNodeDeclaration">
    <LibraryNode>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:attribute name="Name">
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:for-each select="Interface">
        <xsl:apply-templates />
      </xsl:for-each>
    </LibraryNode>
  </xsl:template>

  <xsl:template match="Parameter">
    <Parameter>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:if test="Name">
        <xsl:attribute name="Name">
          <xsl:value-of select="Name" />
        </xsl:attribute>
      </xsl:if>
      <xsl:attribute name="Type">
        <xsl:value-of select="Type" />
        <xsl:if test="MaxSize">
          <xsl:text>Array</xsl:text>
        </xsl:if>
      </xsl:attribute>
      <xsl:if test="MaxSize">
        <xsl:attribute name="MaxSize">
          <xsl:value-of select="MaxSize" />
        </xsl:attribute>
      </xsl:if>
    </Parameter>
  </xsl:template>

  <xsl:template match="DeclareMutex">
    <DeclareMutex>
      <xsl:attribute name="Name">
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
    </DeclareMutex>
  </xsl:template>

  <xsl:template match="Node">
    <xsl:choose>
      <xsl:when test="@NodeType eq 'Assignment'">
        <xsl:call-template name="Assignment" />
      </xsl:when>
      <xsl:when test="@NodeType eq 'Command'">
        <xsl:call-template name="CommandNode" />
      </xsl:when>
      <xsl:when test="@NodeType eq 'Empty'">
        <xsl:call-template name="EmptyNode" />
      </xsl:when>
      <xsl:when test="@NodeType eq 'LibraryNodeCall'">
        <xsl:call-template name="LibraryCall" />
      </xsl:when>
      <xsl:when test="@NodeType eq 'NodeList'">
        <xsl:call-template name="ListNode" />
      </xsl:when>
      <xsl:when test="@NodeType eq 'Update'">
        <xsl:call-template name="UpdateNode" />
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="NodeCommon">
    <xsl:attribute name="NodeId">
      <xsl:value-of select="NodeId" />
    </xsl:attribute>
    <xsl:if test="Priority">
      <xsl:attribute name="Priority">
        <xsl:value-of select="Priority" />
      </xsl:attribute>
    </xsl:if>
    <xsl:copy-of select="@epx|@FileName|@LineNo|@ColNo" />
    <xsl:apply-templates select="Comment" />
    <xsl:for-each select="EndCondition|ExitCondition|InvariantCondition|PostCondition|
                          PreCondition|RepeatCondition|SkipCondition|StartCondition">
      <xsl:element name="{name()}">
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
        <xsl:apply-templates select="*" />
      </xsl:element>
    </xsl:for-each> 
    <xsl:for-each select="Interface/*">
      <xsl:element name="{name()}">
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
        <xsl:for-each select=".">
          <xsl:apply-templates />
        </xsl:for-each>
      </xsl:element>
    </xsl:for-each>
    <xsl:for-each select="VariableDeclarations"> 
      <xsl:apply-templates/> 
    </xsl:for-each> 
    <xsl:apply-templates select="UsingMutex" />
  </xsl:template>

  <xsl:template name="Assignment">
    <Assignment>
      <xsl:call-template name="NodeCommon" />
      <xsl:apply-templates select="NodeBody/Assignment/*[1]" />
      <xsl:apply-templates
          select="NodeBody/Assignment/(BooleanRHS|NumericRHS|StringRHS|ArrayRHS)/*" />
    </Assignment>
  </xsl:template>

  <xsl:template name="CommandNode">
    <CommandNode>
      <xsl:call-template name="NodeCommon" />
      <xsl:apply-templates select="NodeBody/Command" />
    </CommandNode>
  </xsl:template>

  <xsl:template match="Command">
    <Command>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:apply-templates select="ResourceList" />
      <xsl:apply-templates select="ArrayElement|ArrayVariable|BooleanVariable|
                                   IntegerVariable|RealVariable|StringVariable" />
      <Name>
        <xsl:apply-templates select="Name/*" />
      </Name>
      <xsl:for-each select="Arguments" >
        <xsl:apply-templates />
      </xsl:for-each>
    </Command>
  </xsl:template>

  <xsl:template match="ResourceList">
    <xsl:for-each select="Resource" >
      <Resource>
        <Name>
          <xsl:apply-templates select="ResourceName/*" />
        </Name>
        <Priority>
          <xsl:apply-templates select="ResourcePriority/*" />
        </Priority>
        <xsl:if test="ResourceUpperBound">
          <UpperBound>
            <xsl:apply-templates select="ResourceUpperBound/*" />
          </UpperBound>
        </xsl:if>
        <xsl:if test="ResourceReleaseAtTermination">
          <ReleaseAtTermination>
            <xsl:apply-templates select="ResourceReleaseAtTermination/*" />
          </ReleaseAtTermination>
        </xsl:if>
      </Resource>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="EmptyNode">
    <EmptyNode>
      <xsl:call-template name="NodeCommon" />
    </EmptyNode>
  </xsl:template>

  <xsl:template name="LibraryCall">
    <LibraryCall>
      <xsl:call-template name="NodeCommon" />
      <Library>
        <xsl:value-of select="NodeBody/LibraryNodeCall/NodeId" />
      </Library>
      <xsl:apply-templates select="NodeBody/LibraryNodeCall/Alias" />
    </LibraryCall>
  </xsl:template>

  <xsl:template match="Alias">
    <Alias>
      <xsl:attribute name="NodeParameter">
        <xsl:value-of select="NodeParameter" />
      </xsl:attribute>
      <xsl:apply-templates select='*[2]' />
    </Alias>
  </xsl:template>

  <xsl:template name="ListNode">
    <ListNode>
      <xsl:call-template name="NodeCommon" />
      <xsl:for-each select="NodeBody/NodeList">
        <xsl:apply-templates />
      </xsl:for-each>
    </ListNode>
  </xsl:template>

  <xsl:template name="UpdateNode">
    <UpdateNode>
      <xsl:call-template name="NodeCommon" />
      <xsl:apply-templates select="NodeBody/Update" />
    </UpdateNode>
  </xsl:template>

  <xsl:template match="Update">
    <Update>
      <xsl:for-each select="Pair">
        <Pair>
          <xsl:attribute name="Name">
            <xsl:value-of select="Name" />
          </xsl:attribute>
          <xsl:apply-templates select="*[2]" />
        </Pair>
      </xsl:for-each>
    </Update>
  </xsl:template>

  <!-- Variable declarations -->

  <xsl:template match="DeclareVariable">
    <DeclareVariable>
      <xsl:attribute name="Name" >
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:attribute name="Type">
        <xsl:value-of select="Type" />
      </xsl:attribute>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:apply-templates select="InitialValue/*" />
    </DeclareVariable>
  </xsl:template>

  <xsl:template match="DeclareArray">
    <DeclareArray>
      <xsl:attribute name="Name" >
        <xsl:value-of select="Name" />
      </xsl:attribute>
      <xsl:attribute name="ElementType">
        <xsl:value-of select="Type" />
      </xsl:attribute>
      <xsl:attribute name="MaxSize">
        <xsl:value-of select="MaxSize" />
      </xsl:attribute>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:if test="InitialValue">
        <xsl:choose>
          <xsl:when test="key('LiteralValue', InitialValue/*)">
            <!-- Old style initial value -->
            <Array>
              <xsl:attribute name="ElementType">
                <xsl:value-of select="Type" />
              </xsl:attribute>
              <xsl:for-each select="InitialValue">
                <xsl:apply-templates />
              </xsl:for-each>
            </Array>
          </xsl:when>
          <xsl:otherwise>
            <xsl:apply-templates select="InitialValue/*" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </DeclareArray>
  </xsl:template>

  <!-- UsingMutex -->

  <xsl:template match="UsingMutex">
    <UsingMutex>
      <xsl:for-each select="Name">
        <Mutex>
          <xsl:attribute name="Name">
            <xsl:value-of select="." />
          </xsl:attribute>
          <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
        </Mutex>
      </xsl:for-each>
    </UsingMutex>
  </xsl:template>

  <!-- Expressions -->

  <xsl:template match="ArrayVariable|BooleanVariable|IntegerVariable|RealVariable|StringVariable">
    <xsl:element name="{name()}">
      <xsl:attribute name="Name">
        <xsl:value-of select="." />
      </xsl:attribute>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="ArrayElement">
    <ArrayElement>
      <xsl:choose>
        <xsl:when test="Name">
          <ArrayVariable>
            <xsl:attribute name="Name">
              <xsl:value-of select="Name" />
            </xsl:attribute>
          </ArrayVariable>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="*[1]" />
        </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="Index/*" />
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
    </ArrayElement>
  </xsl:template>

  <xsl:template match="LookupOnChange">
    <Lookup>
      <xsl:if test="Tolerance">
        <Tolerance>
          <xsl:apply-templates select="Tolerance/*" />
        </Tolerance>
      </xsl:if>
      <xsl:apply-templates select="Name" />
      <xsl:apply-templates select="Arguments/*" />
    </Lookup>
  </xsl:template>

  <xsl:template match="LookupNow">
    <Lookup>
      <xsl:apply-templates select="Name" />
      <xsl:apply-templates select="Arguments/*" />
    </Lookup>
  </xsl:template>

  <xsl:template match="NodeTimepointValue">
    <NodeTimepoint>
      <xsl:attribute name="NodeState">
        <xsl:value-of select="NodeStateValue" />
      </xsl:attribute>
      <xsl:attribute name="Timepoint">
        <xsl:value-of select="Timepoint" />
      </xsl:attribute>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:apply-templates select="NodeId|NodeRef"/>
    </NodeTimepoint>
  </xsl:template>

  <xsl:template match="BooleanValue">
    <Boolean>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </Boolean>
  </xsl:template>

  <xsl:template match="IntegerValue">
    <Integer>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </Integer>
  </xsl:template>

  <xsl:template match="RealValue">
    <Real>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </Real>
  </xsl:template>

  <xsl:template match="StringValue">
    <String>
      <xsl:value-of select="text()" />
    </String>
  </xsl:template>

  <xsl:template match="ArrayValue">
    <Array>
      <xsl:attribute name="ElementType">
        <xsl:value-of select="@Type" />
      </xsl:attribute>
      <xsl:for-each select=".">
        <xsl:apply-templates />
      </xsl:for-each>
    </Array>
  </xsl:template>

  <xsl:template match="NodeStateValue">
    <NodeState>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </NodeState>
  </xsl:template>

  <xsl:template match="NodeOutcomeValue">
    <NodeOutcome>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </NodeOutcome>
  </xsl:template>

  <xsl:template match="NodeFailureValue">
    <NodeFailure>
      <xsl:attribute name="value">
        <xsl:value-of select="text()" />
      </xsl:attribute>
    </NodeFailure>
  </xsl:template>

</xsl:stylesheet>
