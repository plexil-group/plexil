<?xml version="1.0" encoding="ISO-8859-1"?>

<!--
* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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
                >

  <xsl:output method="xml" indent="yes"/> <!-- indent="no" when debugged -->

  <!-- Selector keys -->

  <xsl:key name="NodeTypes"
           match="EmptyNode|Assignment|CommandNode|LibraryCall|ListNode|UpdateNode"
           use="." />

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

  <xsl:key name="SourceLocators"
           match="@FileName|@LineNo|@ColNo"
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
    <PlexilPlan>
      <xsl:copy-of select="@FileName" />
      <!-- 0 or 1 expected -->
      <xsl:apply-templates select="GlobalDeclarations" />
      <!-- 1 expected -->
      <xsl:apply-templates select="EmptyNode|Assignment|CommandNode|LibraryCall|ListNode|UpdateNode" />
    </PlexilPlan>
  </xsl:template>

  <xsl:template match="GlobalDeclarations">
    <GlobalDeclarations>
      <xsl:apply-templates />
    </GlobalDeclarations>
  </xsl:template>

  <xsl:template match="CommandDeclaration">
    <CommandDeclaration>
      <xsl:copy-of select="key('SourceLocators', .)" />
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
      <xsl:if test="@Type">
        <xsl:call-template name="Return" />
      </xsl:if>
      <xsl:apply-templates select="Parameter" />
      <xsl:apply-templates select="AnyParameters" />
      <xsl:apply-templates select="ResourceList" />
    </CommandDeclaration>
  </xsl:template>

  <xsl:template match="StateDeclaration">
    <StateDeclaration>
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
      <xsl:call-template name="Return" />
      <xsl:apply-templates select="Parameter"/>
      <xsl:apply-templates select="AnyParameters" />
    </StateDeclaration>
  </xsl:template>

  <xsl:template name="Return">
    <xsl:variable name="isArrayType"
                  select="fn:ends-with(@Type, 'Array')" />
    <Return>
      <Type>
        <xsl:choose>
          <xsl:when test="$isArrayType">
            <xsl:value-of select="fn:substring-before(@Type, 'Array')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@Type" />
          </xsl:otherwise>
        </xsl:choose>
      </Type>
      <xsl:if test="$isArrayType">
        <MaxSize>
          <xsl:choose>
            <xsl:when test="@MaxSize">
              <xsl:value-of select="@MaxSize" />
            </xsl:when>
            <xsl:otherwise>
              <xsl:text>1000</xsl:text> <!-- pick a random default value -->
            </xsl:otherwise>
          </xsl:choose>
        </MaxSize>
      </xsl:if>
    </Return>
  </xsl:template>

  <xsl:template match="LibraryNode">
    <LibraryNodeDeclaration>
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
      <xsl:if test="In|InOut">
        <Interface>
          <xsl:apply-templates select="In|InOut" />
        </Interface>
      </xsl:if>
    </LibraryNodeDeclaration>
  </xsl:template>

  <xsl:template match="Parameter">
    <Parameter>
      <xsl:if test="@Name">
        <Name>
          <xsl:value-of select="@Name" />
        </Name>
      </xsl:if>
      <Type>
        <xsl:value-of select="@Type" />
      </Type>
      <xsl:if test="@MaxSize">
        <MaxSize>
          <xsl:value-of select="@MaxSize" />
        </MaxSize>
      </xsl:if>
    </Parameter>
  </xsl:template>

  <xsl:template name="NodeCommon">
    <xsl:copy-of select="@epx|@FileName|@LineNo|@ColNo" />
    <NodeId>
      <xsl:value-of select="@NodeId" />
    </NodeId>
    <xsl:copy-of select="Comment" />
    <xsl:for-each select="EndCondition|ExitCondition|InvariantCondition|PostCondition|
                          PreCondition|RepeatCondition|SkipCondition|StartCondition">
      <xsl:element name="{name()}">
        <xsl:apply-templates select="*" />
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      </xsl:element>
    </xsl:for-each> 
    <xsl:if test="In|InOut">
      <Interface>
        <xsl:for-each select="In|InOut">
          <xsl:element name="{name()}">
            <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
            <xsl:for-each select=".">
              <xsl:apply-templates />
            </xsl:for-each>
          </xsl:element>
        </xsl:for-each>
      </Interface>
    </xsl:if>
    <xsl:if test="DeclareVariable|DeclareArray">
      <VariableDeclarations>
        <xsl:apply-templates select="DeclareVariable|DeclareArray" /> 
      </VariableDeclarations>
    </xsl:if>
  </xsl:template>

  <xsl:template match="EmptyNode">
    <Node NodeType="Empty">
      <xsl:call-template name="NodeCommon" />
    </Node>
  </xsl:template>

  <xsl:template match="Assignment">
    <Node NodeType="Assignment">
      <xsl:call-template name="NodeCommon" />
      <xsl:if test="@Priority">
        <Priority>
          <xsl:value-of select="@Priority" />
        </Priority>
      </xsl:if>
      <NodeBody>
        <Assignment>
          <xsl:variable name="var" as="element()"> <!-- XSL 2.0 feature -->
            <xsl:apply-templates select="(ArrayElement|ArrayVariable|BooleanVariable|
                                         IntegerVariable|RealVariable|StringVariable)[1]" />
          </xsl:variable>
          <xsl:variable name="expr">
            <xsl:apply-templates select="*[fn:last()]" />
          </xsl:variable>
          <xsl:copy-of select="$var" />
          <xsl:choose>
            <xsl:when test="fn:ends-with(fn:name($var), 'Variable')">
              <xsl:variable name="rhsElt" as="xs:string">
                <xsl:call-template name="rhsName">
                  <xsl:with-param name="typeString" select="fn:name($var)" />
                </xsl:call-template>
              </xsl:variable>
              <xsl:element name="{$rhsElt}">
                <xsl:copy-of select="$expr" />
              </xsl:element>
            </xsl:when>
            <xsl:when test="fn:name($var) = 'ArrayElement'">
              <xsl:choose>
                <!-- If array is var ref, try to find declaration to extract type -->
                <xsl:when test="ArrayElement/ArrayVariable">
                  <xsl:variable name="decl" as="element()?">
                    <xsl:call-template name="findArrayDeclaration">
                      <xsl:with-param name="varName">
                        <xsl:value-of select="ArrayElement/ArrayVariable/@Name" />
                      </xsl:with-param>
                      <xsl:with-param name="context" select="."/>
                    </xsl:call-template>
                  </xsl:variable>
                  <xsl:choose>
                    <xsl:when test="$decl/@ElementType">
                      <xsl:variable name="rhsElt" as="xs:string">
                        <xsl:call-template name="rhsName">
                          <xsl:with-param name="typeString" select="$decl/@ElementType" />
                        </xsl:call-template>
                      </xsl:variable>
                      <xsl:element name="{$rhsElt}">
                        <xsl:copy-of select="$expr" />
                      </xsl:element>
                    </xsl:when>
                    <!-- DEBUG -->
                    <xsl:otherwise>
                      <xsl:copy-of select="$decl"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:when>
                <xsl:otherwise>
                  <GenericRHS>
                    <xsl:copy-of select="$expr" />
                  </GenericRHS>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <!-- No idea, punt -->
              <GenericRHS>
                <xsl:copy-of select="$expr" />
              </GenericRHS>
            </xsl:otherwise>
          </xsl:choose>
        </Assignment>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template match="CommandNode">
    <Node NodeType="Command">
      <xsl:call-template name="NodeCommon" />
      <xsl:apply-templates select="Command" />
    </Node>
  </xsl:template>

  <xsl:template match="Command">
    <NodeBody>
      <Command>
        <xsl:if test="Resource">
          <ResourceList>
            <xsl:apply-templates select="Resource" />
          </ResourceList>
        </xsl:if>
        <xsl:apply-templates select="(ArrayElement|ArrayVariable|BooleanVariable|
                                     IntegerVariable|RealVariable|StringVariable)[following-sibling::Name]" />
        <Name>
          <xsl:apply-templates select="Name/*" />
        </Name>
        <!-- Everything after Name is an argument -->
        <xsl:if test="*[preceding-sibling::Name]" >
          <Arguments>
            <xsl:apply-templates select="*[preceding-sibling::Name]" />
          </Arguments>
        </xsl:if>
      </Command>
    </NodeBody>
  </xsl:template>

  <xsl:template match="Resource" >
    <Resource>
      <ResourceName>
        <xsl:apply-templates select="Name/*" />
      </ResourceName>
      <ResourcePriority>
        <xsl:apply-templates select="Priority/*" />
      </ResourcePriority>
      <xsl:if test="LowerBound">
        <ResourceLowerBound>
          <xsl:apply-templates select="LowerBound/*" />
        </ResourceLowerBound>
      </xsl:if>
      <xsl:if test="UpperBound">
        <ResourceUpperBound>
          <xsl:apply-templates select="UpperBound/*" />
        </ResourceUpperBound>
      </xsl:if>
      <xsl:if test="ReleaseAtTermination">
        <ResourceReleaseAtTermination>
          <xsl:apply-templates select="ReleaseAtTermination/*" />
        </ResourceReleaseAtTermination>
      </xsl:if>
    </Resource>
  </xsl:template>

  <xsl:template match="LibraryCall">
    <Node NodeType="LibraryNodeCall">
      <xsl:call-template name="NodeCommon" />
      <NodeBody>
        <LibraryNodeCall>
          <NodeId>
            <xsl:value-of select="Library" />
          </NodeId>
          <xsl:apply-templates select="Alias" />
        </LibraryNodeCall>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template match="Alias">
    <Alias>
      <NodeParameter>
        <xsl:value-of select="@NodeParameter" />
      </NodeParameter>
      <xsl:apply-templates select='*' />
    </Alias>
  </xsl:template>

  <xsl:template match="ListNode">
    <Node NodeType="NodeList">
      <xsl:call-template name="NodeCommon" />
      <NodeBody>
        <NodeList>
          <xsl:apply-templates select="EmptyNode|Assignment|CommandNode|LibraryCall|ListNode|UpdateNode" />
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template match="UpdateNode">
    <Node NodeType="Update">
      <xsl:call-template name="NodeCommon" />
      <xsl:apply-templates select="Update" />
    </Node>
  </xsl:template>

  <xsl:template match="Update">
    <NodeBody>
      <Update>
        <xsl:for-each select="Pair">
          <Pair>
            <Name>
              <xsl:value-of select="@Name" />
            </Name>
            <xsl:apply-templates select="*" />
          </Pair>
        </xsl:for-each>
      </Update>
    </NodeBody>
  </xsl:template>

  <!-- Variable declarations -->

  <xsl:template match="DeclareVariable">
    <DeclareVariable>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
      <Type>
        <xsl:value-of select="@Type" />
      </Type>
      <xsl:if test="*">
        <InitialValue>
          <xsl:apply-templates select="*" />
        </InitialValue>
      </xsl:if>
    </DeclareVariable>
  </xsl:template>

  <xsl:template match="DeclareArray">
    <DeclareArray>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
      <Type>
        <xsl:value-of select="@ElementType" />
      </Type>
      <xsl:if test="@MaxSize">
        <MaxSize>
          <xsl:value-of select="@MaxSize" />
        </MaxSize>
      </xsl:if>
      <xsl:if test="*">
        <InitialValue>
          <xsl:choose>
            <xsl:when test="Array">
              <ArrayValue>
                <xsl:attribute name="Type">
                  <xsl:value-of select="@ElementType" />
                </xsl:attribute>
                <xsl:for-each select="Array">
                  <xsl:apply-templates />
                </xsl:for-each>
              </ArrayValue>
            </xsl:when>
            <xsl:otherwise>
              <xsl:apply-templates select="*[fn:last()]" />
            </xsl:otherwise>
          </xsl:choose>
        </InitialValue>
      </xsl:if>
    </DeclareArray>
  </xsl:template>

  <!-- Expressions -->

  <xsl:template match="ArrayVariable|BooleanVariable|IntegerVariable|RealVariable|StringVariable">
    <xsl:element name="{name()}">
      <xsl:value-of select="@Name" />
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="ArrayElement">
    <ArrayElement>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:choose>
        <xsl:when test="ArrayVariable">
          <Name>
              <xsl:value-of select="ArrayVariable/@Name" />
          </Name>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="*[1]" />
        </xsl:otherwise>
      </xsl:choose>
      <Index>
        <xsl:apply-templates select="*[2]" />
      </Index>
    </ArrayElement>
  </xsl:template>

  <xsl:template match="LookupOnChange">
    <LookupOnChange>
      <xsl:apply-templates select="Name" />
      <xsl:if test="Tolerance">
        <Tolerance>
          <xsl:apply-templates select="Tolerance/*" />
        </Tolerance>
      </xsl:if>
      <xsl:if test="*[preceding-sibling::Name]">
        <Arguments>
          <xsl:apply-templates select="*[preceding-sibling::Name]" />
        </Arguments>
      </xsl:if>
    </LookupOnChange>
  </xsl:template>

  <xsl:template match="LookupNow">
    <LookupNow>
      <xsl:apply-templates select="Name" />
      <xsl:if test="*[preceding-sibling::Name]">
        <Arguments>
          <xsl:apply-templates select="*[preceding-sibling::Name]" />
        </Arguments>
      </xsl:if>
    </LookupNow>
  </xsl:template>

  <xsl:template match="NodeTimepoint">
    <NodeTimepointValue>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:copy-of select="NodeId|NodeRef" />
      <NodeStateValue>
        <xsl:value-of select="@NodeState" />
      </NodeStateValue>
      <Timepoint>
        <xsl:value-of select="@Timepoint" />
      </Timepoint>
    </NodeTimepointValue>
  </xsl:template>

  <xsl:template match="Boolean">
    <BooleanValue>
      <xsl:value-of select="@value" />
    </BooleanValue>
  </xsl:template>

  <xsl:template match="Integer">
    <IntegerValue>
      <xsl:value-of select="@value" />
    </IntegerValue>
  </xsl:template>

  <xsl:template match="Real">
    <RealValue>
      <xsl:value-of select="@value" />
    </RealValue>
  </xsl:template>

  <xsl:template match="String">
    <StringValue>
      <xsl:value-of select="text()" />
    </StringValue>
  </xsl:template>

  <xsl:template match="Array">
    <ArrayValue>
      <xsl:attribute name="Type">
        <xsl:value-of select="@ElementType" />
      </xsl:attribute>
      <xsl:for-each select=".">
        <xsl:apply-templates />
      </xsl:for-each>
    </ArrayValue>
  </xsl:template>

  <xsl:template match="NodeState">
    <NodeStateValue>
      <xsl:value-of select="@value" />
    </NodeStateValue>
  </xsl:template>

  <xsl:template match="NodeOutcome">
    <NodeOutcomeValue>
      <xsl:value-of select="@value" />
    </NodeOutcomeValue>
  </xsl:template>

  <xsl:template match="NodeFailure">
    <NodeFailureValue>
      <xsl:value-of select="@value" />
    </NodeFailureValue>
  </xsl:template>

  <!-- Utility -->

  <xsl:template name="findArrayDeclaration">
    <xsl:param name="varName" as="xs:string" />
    <xsl:param name="context" as="element()" /> <!-- Should be PLEXIL Node containing the ArrayElement -->
    <xsl:variable name="localVarDecl"
                  select="$context/DeclareArray[@Name=$varName]" />
    <xsl:choose>
      <xsl:when test="$localVarDecl">
        <xsl:copy-of select="$localVarDecl" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="localIntfcDecl"
                      select="$context/(In|InOut)/DeclareArray[@Name=$varName]"
                      />
        <xsl:choose>
          <xsl:when test="$localIntfcDecl">
            <xsl:copy-of select="$localIntfcDecl" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:variable name="parent"
                          select="$context/.." />
            <xsl:choose>
              <xsl:when test="$parent">
                <xsl:call-template name="findArrayDeclaration">
                  <xsl:with-param name="varName" select="$varName"/>
                  <xsl:with-param name="context" select="$parent" />
                </xsl:call-template>
              </xsl:when>
              <!-- debug -->
              <xsl:otherwise>
                <NotFound>
                  <xsl:attribute name="varName" select="$varName" />
                  <Context>
                    <xsl:element name="{name($context)}">
                      <xsl:copy-of select="$context/@*" />
                    </xsl:element>
                  </Context>
                </NotFound>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="rhsName">
    <xsl:param name="typeString" as="xs:string?" />
    <xsl:choose>
      <xsl:when test="fn:starts-with($typeString, 'Array')">
        <xsl:text>ArrayRHS</xsl:text>
      </xsl:when>
      <xsl:when test="fn:starts-with($typeString, 'Boolean')">
        <xsl:text>BooleanRHS</xsl:text>
      </xsl:when>
      <xsl:when test="fn:starts-with($typeString, 'Integer')">
        <xsl:text>NumericRHS</xsl:text>
      </xsl:when>
      <xsl:when test="fn:starts-with($typeString, 'Real')">
        <xsl:text>NumericRHS</xsl:text>
      </xsl:when>
      <xsl:when test="fn:starts-with($typeString, 'String')">
        <xsl:text>StringRHS</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>UnknownRHS</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
