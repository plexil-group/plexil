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

<!-- The purpose of this stylesheet is to put a valid Core PLEXIL v1
plan into a canonical form, which can be compared directly with another
canonicalized plan to determine if they are equivalent. -->

<xsl:stylesheet version="2.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fn="http://www.w3.org/2005/xpath-functions"
                xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                exclude-result-prefixes="fn xsi xsl"
                >
  <!-- xmlns="https://svn.code.sf.net/p/plexil/code/branches/plexil-x/schema/core-plexil.xsd" -->
  <xsl:output method="xml" indent="yes"/>

  <xsl:preserve-space elements="StringValue" />
  <xsl:strip-space elements="*" />

  <!-- Selector keys -->

  <xsl:key name="SourceLocators"
           match="@FileName|@LineNo|@ColNo"
           use="." />

  <!-- Copy most attributes -->
  <xsl:template match="@*">
    <xsl:copy />
  </xsl:template>

  <!-- Don't pass this attribute in output -->
  <xsl:template match="@xsi:noNamespaceSchemaLocation"/>

  <!-- <xsl:template match="text() | processing-instruction()"> -->
  <!--   <xsl:copy /> -->
  <!-- </xsl:template> -->

  <!-- Don't copy comments -->
  <xsl:template match="comment()" />

  <!-- Generic element template -->
  <xsl:template match="element()">
    <xsl:element name="{name()}" namespace="{namespace-uri()}">
      <xsl:copy-of select="namespace::*[not(. = 'http://www.w3.org/2001/XMLSchema-instance')
                           and not(. = 'extended-plexil-translator') ]" />

      <!-- Put attributes in sorted order -->
      <xsl:perform-sort> <!-- XSLT 2.0 feature -->
        <xsl:sort select="fn:name(.)" />
        <xsl:apply-templates select="@* "/>
      </xsl:perform-sort>
      
      <xsl:apply-templates select="node()" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="Node">
    <Node>
      <!-- Put attributes in sorted order -->
      <xsl:perform-sort> <!-- XSLT 2.0 feature -->
        <xsl:sort select="fn:name(.)" />
        <xsl:apply-templates select="@* "/>
      </xsl:perform-sort>

      <xsl:copy-of select="NodeId" copy-namespaces="no" /> <!-- XSLT 2.0 feature -->
      <xsl:copy-of select="Comment" copy-namespaces="no" /> <!-- XSLT 2.0 feature -->
      <xsl:apply-templates select="Interface" />
      <xsl:apply-templates select="VariableDeclarations" />

      <!-- Put conditions into a canonical order -->
      <xsl:apply-templates select="SkipCondition" />
      <xsl:apply-templates select="StartCondition" />
      <xsl:apply-templates select="PreCondition" />
      <xsl:apply-templates select="ExitCondition" />
      <xsl:apply-templates select="InvariantCondition" />
      <xsl:apply-templates select="EndCondition" />
      <xsl:apply-templates select="PostCondition" />
      <xsl:apply-templates select="RepeatCondition" />

      <xsl:copy-of select="Priority" copy-namespaces="no" /> <!-- XSLT 2.0 feature -->
      <xsl:apply-templates select="NodeBody" />
    </Node>
  </xsl:template>

  <xsl:template match="ArrayElement">
    <ArrayElement>
      <!-- Put attributes in sorted order -->
      <xsl:perform-sort> <!-- XSLT 2.0 feature -->
        <xsl:sort select="fn:name(.)" />
        <xsl:apply-templates select="@* "/>
      </xsl:perform-sort>

      <xsl:choose>
        <!-- Convert old-style array variable reference -->
        <xsl:when test="fn:name(*[1]) eq 'Name'">
          <ArrayVariable>
            <xsl:value-of select="Name" />
          </ArrayVariable>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="*[1]" />
        </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*[2]" />
    </ArrayElement>
  </xsl:template>

  <xsl:template match="DeclareArray">
    <DeclareArray>
      <!-- Put attributes in sorted order -->
      <xsl:perform-sort> <!-- XSLT 2.0 feature -->
        <xsl:sort select="fn:name(.)" />
        <xsl:apply-templates select="@* "/>
      </xsl:perform-sort>

      <xsl:copy-of select="Name" copy-namespaces="no" />
      <xsl:copy-of select="Type" copy-namespaces="no" />
      <xsl:copy-of select="MaxSize" copy-namespaces="no"/>

      <xsl:if test="InitialValue">
        <xsl:choose>
          <xsl:when test="InitialValue/ArrayValue">
            <xsl:apply-templates select="InitialValue" />
          </xsl:when>
          
          <!-- Old style initial value -->
          <!-- Wrap initial values in an ArrayValue element -->
          <xsl:otherwise>
            <InitialValue>
              <ArrayValue>
                <xsl:attribute name="Type">
                  <xsl:value-of select="Type" />
                </xsl:attribute>
                <xsl:apply-templates select="InitialValue/*" />
              </ArrayValue>
            </InitialValue>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>

    </DeclareArray>
  </xsl:template>

  <!-- Put children of Resource element into a specific order -->
  <xsl:template match="Resource">
    <Resource>
      <!-- Put attributes in sorted order -->
      <xsl:perform-sort> <!-- XSLT 2.0 feature -->
        <xsl:sort select="fn:name(.)" />
        <xsl:apply-templates select="@* "/>
      </xsl:perform-sort>
      <xsl:apply-templates select="ResourceName" />
      <xsl:apply-templates select="ResourceUpperBound" />
      <xsl:apply-templates select="ResourceReleaseAtTermination" />
    </Resource>
  </xsl:template>

  <!-- Strip Name element from Return, as it is ignored -->
  <xsl:template match="Return">
    <Return>
      <xsl:copy-of select="Type" copy-namespaces="no" />
      <xsl:copy-of select="MaxSize" copy-namespaces="no" />
    </Return>
  </xsl:template>

  <!-- Strip source locators from Assignment -->
  <!-- This kludge works around the inability to copy them into PLEXIL V2 XML -->
  <xsl:template match="Assignment">
    <Assignment>
      <xsl:apply-templates select="*" />
    </Assignment>
  </xsl:template>

</xsl:stylesheet>
