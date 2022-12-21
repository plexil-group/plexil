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

  <xsl:preserve-space elements="String" />
  <xsl:strip-space elements="*" />

  <!-- Selector keys -->

  <xsl:key name="SourceLocators"
           match="@FileName|@LineNo|@ColNo"
           use="." />

  <!-- This is the "overriding copy idiom", from "XSLT Cookbook" by
       Sal Mangano.  It is the identity transform, covering all
       elements that are not explicitly handled elsewhere. -->

  <xsl:template match="@* | node()">
    <xsl:copy copy-namespaces="no"> <!-- XSLT 2.0 feature -->
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
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <xsl:apply-templates />
    </GlobalDeclarations>
  </xsl:template>

  <xsl:template match="CommandDeclaration">
    <CommandDeclaration>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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

  <xsl:template match="DeclareMutex">
    <DeclareMutex>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
      <Name>
        <xsl:value-of select="@Name" />
      </Name>
    </DeclareMutex>
  </xsl:template>

  <xsl:template name="NodeCommon">
    <xsl:copy-of select="@epx|@FileName|@LineNo|@ColNo" />
    <NodeId>
      <xsl:value-of select="@NodeId" />
    </NodeId>
    <xsl:copy-of select="Comment" />
    <!-- Only these expression contexts allow LookupOnChange -->
    <xsl:for-each select="EndCondition|ExitCondition|InvariantCondition|
                          SkipCondition|StartCondition">
      <xsl:element name="{name()}">
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
        <xsl:apply-templates select="*">
          <xsl:with-param name="lookupMode" tunnel="yes" select="'change'" />
        </xsl:apply-templates>
      </xsl:element>
    </xsl:for-each> 
    <xsl:for-each select="PostCondition|PreCondition|RepeatCondition">
      <xsl:element name="{name()}">
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
        <xsl:apply-templates select="*" />
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
    <xsl:if test="DeclareVariable|DeclareArray|DeclareMutex">
      <VariableDeclarations>
        <xsl:apply-templates select="DeclareVariable|DeclareArray|DeclareMutex" /> 
      </VariableDeclarations>
    </xsl:if>
    <xsl:apply-templates select="UsingMutex" />
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
          <xsl:sequence select="$var" />
          <xsl:choose>
            <xsl:when test="fn:ends-with(fn:name($var), 'Variable')">
              <xsl:variable name="rhsElt" as="xs:string"> <!-- XSL 2.0 feature -->
                <xsl:call-template name="rhsName">
                  <xsl:with-param name="typeString" select="fn:name($var)" />
                </xsl:call-template>
              </xsl:variable>
              <xsl:element name="{$rhsElt}">
                <xsl:sequence select="$expr" />
              </xsl:element>
            </xsl:when>
            <xsl:when test="fn:name($var) = 'ArrayElement'">
              <xsl:choose>
                <!-- If array is var ref, try to find declaration to extract type -->
                <xsl:when test="ArrayElement/ArrayVariable">
                  <xsl:variable name="decl" as="element()?"> <!-- XSL 2.0 feature -->
                    <xsl:call-template name="findArrayDeclaration">
                      <xsl:with-param name="varName">
                        <xsl:value-of select="ArrayElement/ArrayVariable/@Name" />
                      </xsl:with-param>
                    </xsl:call-template>
                  </xsl:variable>
                  <xsl:choose>
                    <xsl:when test="$decl/@ElementType">
                      <xsl:variable name="rhsElt" as="xs:string"> <!-- XSL 2.0 feature -->
                        <xsl:call-template name="rhsName">
                          <xsl:with-param name="typeString" select="$decl/@ElementType" />
                        </xsl:call-template>
                      </xsl:variable>
                      <xsl:element name="{$rhsElt}">
                        <xsl:sequence select="$expr" />
                      </xsl:element>
                    </xsl:when>
                    <xsl:otherwise>
                      <GenericRHS>
                        <xsl:sequence select="$expr" />
                      </GenericRHS>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:when>
                <!-- anything else is invalid -->
                <xsl:otherwise>
                  <xsl:message terminate="yes">
*** Error: Invalid variable expression
<xsl:sequence select="$var" />
in assignment node <xsl:value-of select="@NodeId" />
                    </xsl:message>
                  <GenericRHS>
                    <xsl:sequence select="$expr" />
                  </GenericRHS>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <!-- No idea, issue message -->
              <xsl:message>
*** Error: Unable to determine type of variable expression
<xsl:sequence select="$var" />
in assignment node <xsl:value-of select="@NodeId" />
              </xsl:message>
              <GenericRHS>
                <xsl:sequence select="$expr" />
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
        <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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

  <!-- UsingMutex -->
  <xsl:template match="UsingMutex">
    <UsingMutex>
      <xsl:for-each select="Mutex">
        <Name>
          <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
          <xsl:value-of select="@Name" />
        </Name>
      </xsl:for-each>
    </UsingMutex>
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

  <xsl:template match="Lookup">
    <xsl:param name="lookupMode" tunnel="yes" />
    <xsl:choose>
      <xsl:when test="Tolerance">
        <xsl:call-template name="LookupOnChange" />
      </xsl:when>
      <xsl:when test="$lookupMode='change'">
        <xsl:call-template name="LookupOnChange" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="LookupNow" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="LookupOnChange">
    <LookupOnChange>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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

  <xsl:template name="LookupNow">
    <LookupNow>
      <xsl:copy-of select="@FileName|@LineNo|@ColNo" />
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
    <xsl:param name="varName" as="xs:string" /> <!-- XSL 2.0 feature -->
    <!-- 2nd parameter for recursive calls -->
    <xsl:param name="context" as="element()" required="no" select="." />
    <xsl:variable name="localVarDecl"
                  select="$context/DeclareArray[@Name=$varName]"
                  as="element()?" />
    <xsl:choose>
      <xsl:when test="$localVarDecl">
        <xsl:sequence select="$localVarDecl" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="localIntfcDecl"
                      select="$context/(In|InOut)/DeclareArray[@Name=$varName]"
                      />
        <xsl:choose>
          <xsl:when test="$localIntfcDecl">
            <xsl:sequence select="$localIntfcDecl" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:variable name="parent"
                          select="$context/.." />
            <xsl:choose>
              <xsl:when test="$parent and not ($parent is /)">
                <xsl:variable name="result" as="element()?">
                  <xsl:call-template name="findArrayDeclaration">
                    <xsl:with-param name="varName" select="$varName"/>
                    <xsl:with-param name="context" select="$parent" />
                  </xsl:call-template>
                </xsl:variable>
                <xsl:choose>
                  <xsl:when test="$result">
                    <xsl:sequence select="$result" />
                  </xsl:when>
                  <xsl:when test="$context is .">
                    <xsl:message>
*** Error: Unable to find declaration of array variable "<xsl:value-of select="$varName" />" in Assignment node <xsl:value-of select="$context/@NodeId" />
                    </xsl:message>
                  </xsl:when>
                  <!-- else do nothing -->
                  <xsl:otherwise />
                </xsl:choose>
              </xsl:when>
              <xsl:otherwise>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="rhsName">
    <xsl:param name="typeString" as="xs:string?" /> <!-- XSL 2.0 feature -->
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
      <!-- this is an error -->
      <xsl:otherwise>
        <xsl:message>
*** Error: cannot determine assignment RHS element for type "<xsl:value-of select="$typeString" />"

        </xsl:message>
        <xsl:text>GenericRHS</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
