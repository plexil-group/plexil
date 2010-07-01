<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:transform version="2.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:sum="plexil-summarizer">

<xsl:output method="text" omit-xml-declaration="yes"/>

<!-- Generates a consise textual summary of a Core PLEXIL XML file -->
<!-- Invoked by the script plexil/bin/summarize-plexil -->

<xsl:variable name="tab" select= "'  '"/>

<xsl:template match="PlexilPlan">
  <xsl:text>Summary of PLEXIL Plan 

</xsl:text>
  <!-- currently, there can only been one node here, the root node -->
  <xsl:for-each select="Node">
    <xsl:call-template name= "node"/>
  </xsl:for-each>
</xsl:template>

<xsl:template name= "node">
  <xsl:param name= "indent" select= "''"/>
  <xsl:value-of select= "concat($indent, sum:nicer-name(@NodeType), 'Node ')"/>
  <xsl:value-of select= "NodeId"/>
  <xsl:text>
</xsl:text>
  <xsl:if test= "@NodeType = 'NodeList'">
    <xsl:for-each select="NodeBody/NodeList/Node">
      <xsl:call-template name= "node">
        <xsl:with-param name= "indent" select= "concat($indent, $tab)"/>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:if>
</xsl:template>

<xsl:function name= "sum:nicer-name">
  <xsl:param name= "name"/>
  <xsl:choose>
    <xsl:when test= "$name = 'NodeList'">
      <xsl:sequence select= "'List'"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:sequence select= "$name"/>      
    </xsl:otherwise>
  </xsl:choose>
</xsl:function>

</xsl:transform>
