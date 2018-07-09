<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:transform version="2.0"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:lookup="lookup">

<!--
* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

<xsl:output method="text" omit-xml-declaration="yes"/>

<xsl:variable name="tab" select= "'  '"/>

<xsl:template match="PlexilPlan">
    <xsl:value-of select="concat('// ', @FileName)"/>
    <xsl:text>&#xA;// Generated via the Plexil decompiler&#xA;&#xA;</xsl:text>
    <xsl:if test="descendant::Command/Name/StringValue[text() = 'ReceiveMessage']">
        <xsl:text>String Command ReceiveMesssage(String);&#xA;</xsl:text>
    </xsl:if>
    <xsl:if test="descendant::Command/Name/StringValue[text() = 'ReceiveCommand']">
        <xsl:text>String Command ReceiveCommand(String);&#xA;</xsl:text>
    </xsl:if>
    <xsl:if test="descendant::Command/Name/StringValue[text() = 'GetParameter']">
        <xsl:text>Any Command GetParameter(String, Integer);&#xA;</xsl:text>
    </xsl:if>
    <xsl:if test="descendant::Command/Name/StringValue[text() = 'SendReturnValue']">
        <xsl:text>Command SendReturnValue(String, ...);&#xA;</xsl:text>
    </xsl:if>
    <xsl:for-each-group select="GlobalDeclarations/*" group-adjacent="name()">
        <xsl:apply-templates select="current-group()"/>
        <xsl:text>&#xA;</xsl:text>
    </xsl:for-each-group>
    <xsl:apply-templates select="Node"/>
</xsl:template>

<xsl:template match="Type">
    <xsl:value-of select="if (text()='Date' or text()='Duration') then 'Real' else ."></xsl:value-of>
</xsl:template>
<xsl:template match="Return">
    <xsl:apply-templates select="Type"/>
    <xsl:if test="MaxSize">
        <xsl:value-of select="concat('[', MaxSize, ']')"/>
    </xsl:if>
    <xsl:text> </xsl:text>
</xsl:template>
<xsl:template match="Parameter">
    <xsl:apply-templates select="Type"/>
    <xsl:if test="MaxSize">
        <xsl:value-of select="concat('[', MaxSize, ']')"/>
    </xsl:if>
    <xsl:value-of select="if (Name) then concat(' ', Name) else ''"/>
</xsl:template>
<xsl:template match="AnyParameters">
    <xsl:text>...</xsl:text>
</xsl:template>
<xsl:template match="StateDeclaration">
    <xsl:apply-templates select="Return"/>
    <xsl:value-of select="'Lookup '"/>
    <xsl:apply-templates select="Name"/>
    <xsl:if test="Parameter">
        <xsl:text>(</xsl:text>
        <xsl:for-each select="Parameter">
            <xsl:value-of select="if (position() = 1) then '' else ', '"/>
            <xsl:apply-templates select="."/>
        </xsl:for-each>
        <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>
<xsl:template match="CommandDeclaration">
    <xsl:apply-templates select="Return"/>
    <xsl:text>Command </xsl:text>
    <xsl:value-of select="Name"/>
    <xsl:if test="Parameter|AnyParameters">
        <xsl:text>(</xsl:text>
        <xsl:for-each select="Parameter|AnyParameters">
            <xsl:value-of select="if (position() = 1) then '' else ', '"/>
            <xsl:apply-templates select="."/>
        </xsl:for-each>
        <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>
<xsl:template match="LibraryNodeDeclaration">
    <xsl:value-of select="concat('LibraryAction ', Name, '(')"/>
    <xsl:for-each select="Interface/*">
        <xsl:value-of select="if (position() = 1) then '' else ', '"/>
        <xsl:apply-templates select="."/>
    </xsl:for-each>
    <xsl:text>);&#xA;</xsl:text>
</xsl:template>
<xsl:template match="In">
    <xsl:param name= "indent" select= "''"/>
    <xsl:param name="join" select="', '"/>
    <xsl:for-each select="DeclareVariable|DeclareArray">
        <xsl:value-of select="if (position() = 1) then '' else $join"/>
        <xsl:value-of select="$indent"/>
        <xsl:text>In </xsl:text>
        <xsl:apply-templates select="."/>
    </xsl:for-each>
</xsl:template>
<xsl:template match="InOut">
    <xsl:param name="join" select="', '"/>
    <xsl:for-each select="DeclareVariable|DeclareArray">
        <xsl:value-of select="if (position() = 1) then '' else $join"/>
        <xsl:text>InOut </xsl:text>
        <xsl:apply-templates select="."/>
    </xsl:for-each>
</xsl:template>

<xsl:template match="Node">
    <xsl:param name= "indent" select= "''"/>
    <xsl:variable name="declarations" select="VariableDeclarations/*"/>
    <xsl:variable name="conditions" select="*[ends-with(name(), 'Condition')]"/>
    <xsl:variable name="body" select="NodeBody/*"/>
    <xsl:variable name="multiline" select="$declarations|$conditions|Priority|Interface|Comment or local-name($body) = 'NodeList'"/>

    <xsl:value-of select="concat($indent, replace(NodeId, '-', '_dash_'), ':')"/>
    <xsl:value-of select="if (local-name($body) = 'NodeList') then ' Concurrence' else ''"/>
    <xsl:value-of select="if ($multiline) then concat('&#xA;', $indent, '{&#xA;') else ''"/>
    <xsl:apply-templates select="Comment"/>
    <xsl:apply-templates select="Priority"/>
    <xsl:if test="Interface">
        <xsl:apply-templates select="Interface">
            <xsl:with-param name= "indent" select= "concat($indent, $tab)"/>
        </xsl:apply-templates>
        <xsl:text>&#xA;</xsl:text>
    </xsl:if>
    <xsl:if test="$declarations">
        <xsl:apply-templates select="$declarations">
            <xsl:with-param name= "indent" select= "concat($indent, $tab)"/>
        </xsl:apply-templates>
        <xsl:text>&#xA;</xsl:text>
    </xsl:if>
    <xsl:if test="$conditions">
        <xsl:apply-templates select="$conditions">
            <xsl:with-param name= "indent" select= "concat($indent, $tab)"/>
        </xsl:apply-templates>
        <xsl:text>&#xA;</xsl:text>
    </xsl:if>
    <xsl:apply-templates select="$body">
        <xsl:with-param name= "indent" select= "if ($multiline) then concat($indent, $tab) else ' '"/>
    </xsl:apply-templates>
    <xsl:value-of select="if ($body or $multiline) then '' else '{ }&#xA;'"/>
    <xsl:value-of select="if ($multiline) then concat($indent, '}&#xA;') else ''"/>
</xsl:template>
<xsl:template match="Priority">
    <xsl:value-of select="concat('Priority ', node(), ';&#xA;')"></xsl:value-of>
</xsl:template>
<xsl:template match="Interface[parent::Node]">
    <xsl:param name= "indent" select= "''"/>
    <xsl:for-each select="*">
        <xsl:apply-templates select=".">
            <xsl:with-param name= "indent" select= "$indent"/>
            <xsl:with-param name="join" select="';&#xA;'"/>
        </xsl:apply-templates>
        <xsl:text>;&#xA;</xsl:text>
    </xsl:for-each>
</xsl:template>
<xsl:template match="Comment">
    <xsl:value-of select="concat('Comment &quot;', node(), '&quot;;&#xA;')"></xsl:value-of>
</xsl:template>

<xsl:template match="NodeList">
    <xsl:param name= "indent" select= "''"/>
    <xsl:for-each select="Node">
            <xsl:value-of select="if (position() = 1) then '' else '&#xA;'"></xsl:value-of>
        <xsl:apply-templates select=".">
            <xsl:with-param name= "indent" select= "$indent"/>
        </xsl:apply-templates>
    </xsl:for-each>
</xsl:template>
<xsl:template match="Command">
    <xsl:param name= "indent" select= "''"/>
    <xsl:apply-templates select="ResourceList/Resource">
        <xsl:with-param name= "indent" select= "$indent"/>
    </xsl:apply-templates>
    <xsl:variable name="value" select="RealVariable|ArrayVariable|IntegerVariable|StringVariable|BooleanVariable|ArrayElement"/>
    <xsl:value-of select="$indent"/>
    <xsl:if test="$value">
        <xsl:apply-templates select="$value"/>
        <xsl:text> = </xsl:text>
    </xsl:if>
    <xsl:apply-templates select="Name"/>
    <xsl:text>(</xsl:text>
    <xsl:apply-templates select="Arguments"/>
    <xsl:text>);&#xA;</xsl:text>
</xsl:template>
<xsl:template match="Resource">
    <xsl:param name= "indent" select= "''"/>
    <xsl:variable name="cindent" select="concat($indent, $tab)"></xsl:variable>
    <xsl:value-of select="concat($indent, 'Resource Name = ')"/>
    <xsl:apply-templates select="ResourceName/*"/>
    <xsl:if test="ResourceLowerBound">
        <xsl:value-of select="concat(',&#xA;', $cindent, 'LowerBound = ')"/>
        <xsl:apply-templates select="ResourceLowerBound/*"/>
    </xsl:if>
    <xsl:if test="ResourceUpperBound">
        <xsl:value-of select="concat(',&#xA;', $cindent, 'UpperBound = ')"/>
        <xsl:apply-templates select="ResourceUpperBound/*"/>
    </xsl:if>
    <xsl:if test="ResourcePriority">
        <xsl:value-of select="concat(',&#xA;', $cindent, 'Priority = ')"/>
        <xsl:apply-templates select="ResourcePriority/*"/>
    </xsl:if>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>
<xsl:template match="Arguments">
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ', '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
</xsl:template>
<xsl:template match="Assignment">
    <xsl:param name= "indent" select= "''"/>
    <xsl:variable name="value" select="RealVariable|ArrayVariable|IntegerVariable|StringVariable|BooleanVariable|ArrayElement"/>
    <xsl:value-of select="$indent"/>
    <xsl:apply-templates select="$value"/>
    <xsl:text> = </xsl:text>
    <xsl:apply-templates select="*[ends-with(name(), 'RHS')]" />
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>
<xsl:template match="LibraryNodeCall">
    <xsl:param name= "indent" select= "''"/>
    <xsl:value-of select="concat($indent, 'LibraryCall ', NodeId)"/>
    <xsl:text>(</xsl:text>
    <xsl:for-each select="Alias">
        <xsl:value-of select="if (position() = 1) then '' else ', '"/>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>);&#xA;</xsl:text>
</xsl:template>
<xsl:template match="Update">
    <xsl:text>Update </xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ', '"/>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Alias">
    <xsl:value-of select="concat(NodeParameter, '=')"/>
    <xsl:apply-templates select="*[not(local-name(.) = 'NodeParameter')]"/>
</xsl:template>
<xsl:template match="Pair">
    <xsl:value-of select="concat(Name, ' = ')"/>
    <xsl:apply-templates select="*[not(local-name(.) = 'Name')]"/>
</xsl:template>

<xsl:template match="DeclareVariable[local-name(..) = 'VariableDeclarations']">
    <xsl:param name= "indent" select= "''"/>
    <xsl:value-of select="$indent"/>
    <xsl:apply-templates select="Type"/>
    <xsl:value-of select="concat(' ', Name)"/>
    <xsl:if test="InitialValue">
        <xsl:text> = </xsl:text>
        <xsl:apply-templates select="InitialValue/*"/>
    </xsl:if>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>

<xsl:template match="DeclareVariable">
    <xsl:apply-templates select="Type"/>
    <xsl:value-of select="concat(' ', Name)"/>
</xsl:template>

<xsl:template match="DeclareArray[local-name(..) = 'VariableDeclarations' or ancestor::Interface]">
    <xsl:param name= "indent" select= "''"/>
    <xsl:value-of select="$indent"/>
    <xsl:apply-templates select="Type"/>
    <xsl:value-of select="concat(' ', Name, '[', MaxSize, ']')"/>
    <xsl:if test="InitialValue">
        <xsl:text> = #(</xsl:text>
        <xsl:for-each select="InitialValue/*">
            <xsl:value-of select="if (position() = 1) then '' else ' '"></xsl:value-of>
            <xsl:apply-templates select="." />
        </xsl:for-each>
        <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:if test="local-name(..) = 'VariableDeclarations'">
        <xsl:text>;&#xA;</xsl:text>
    </xsl:if>
</xsl:template>

<xsl:template match="DeclareArray">
    <xsl:apply-templates select="Type"/>
    <xsl:value-of select="concat(' ', Name, '[', MaxSize, ']')"/>
</xsl:template>

<xsl:template match="StringValue">
    <xsl:value-of select="concat('&quot;', replace(node(), '&quot;', '\\&quot;'), '&quot;')"/>
</xsl:template>
<!-- <xsl:template match="RealValue[@epx='DurationValue']">
    <xsl:value-of select="concat('Duration(&quot;', @epx-val, '&quot;)')"/>
</xsl:template>
<xsl:template match="RealValue[@epx='DateValue']">
    <xsl:value-of select="concat('Date(&quot;', @epx-val, '&quot;)')"/>
</xsl:template> -->
<xsl:template match="IntegerValue|BooleanValue|RealValue|NodeOutcomeValue|NodeStateValue|NodeFailureValue|NodeCommandHandleValue">
    <xsl:value-of select="node()"/>
</xsl:template>
<xsl:template match="ArrayValue">
    <xsl:text>#(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="RealVariable|ArrayVariable|IntegerVariable|StringVariable|BooleanVariable">
    <xsl:value-of select="node()"/>
</xsl:template>
<xsl:template match="ArrayElement">
    <xsl:apply-templates select="ArrayVariable|Name|LookupNow|LookupOnChange"/>
    <xsl:text>[</xsl:text>
    <xsl:apply-templates select="Index/*"/>
    <xsl:text>]</xsl:text>
</xsl:template>

<xsl:template match="*[ends-with(name(), 'RHS')]">
    <xsl:apply-templates select="*" />
</xsl:template>

<xsl:template match="*[ends-with(name(), 'Condition')]">
    <xsl:param name= "indent" select= "''"/>
    <xsl:value-of select="concat($indent, local-name(.), ' ')"/>
    <xsl:apply-templates select="*"/>
    <xsl:text>;&#xA;</xsl:text>
</xsl:template>

<xsl:template match="AND">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' &amp;&amp; '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="OR">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' || '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="XOR">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' XOR '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="NOT">
    <xsl:text>!</xsl:text>
    <xsl:apply-templates select="*" />
</xsl:template>
<xsl:template match="GT">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' &gt; '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="GE">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' &gt;= '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="LT">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' &lt; '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="LE">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' &lt;= '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="IsKnown">
    <xsl:text>isKnown(</xsl:text>
    <xsl:apply-templates select="*" />
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="Name">
    <xsl:value-of select="if (StringValue) then
                            (if (contains(StringValue, ':')) then concat('(&quot;', StringValue, '&quot;)') else StringValue)
                            else node()"/>
</xsl:template>
<xsl:template match="LookupOnChange|LookupNow">
    <xsl:text>Lookup(</xsl:text>
    <xsl:apply-templates select="Name"/>
    <xsl:if test="Arguments">
        <xsl:text>(</xsl:text>
        <xsl:apply-templates select="Arguments"/>
        <xsl:text>)</xsl:text>
    </xsl:if>
    <xsl:if test="Tolerance">
        <xsl:text>, </xsl:text>
        <xsl:apply-templates select="Tolerance/*"/>
    </xsl:if>
    <xsl:text>)</xsl:text>
</xsl:template>

<xsl:template match="EQNumeric|EQString|EQBoolean|EQArray|EQInternal">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' == '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="NENumeric|NEString|NEBoolean|NEArray|NEInternal">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' != '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="ADD|Concat">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' + '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="SUB">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' - '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="MUL">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' * '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="DIV">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' / '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="MOD">
    <xsl:text>(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ' % '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="MIN">
    <xsl:text>min(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ', '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>
<xsl:template match="MAX">
    <xsl:text>max(</xsl:text>
    <xsl:for-each select="*">
        <xsl:value-of select="if (position() = 1) then '' else ', '"></xsl:value-of>
        <xsl:apply-templates select="." />
    </xsl:for-each>
    <xsl:text>)</xsl:text>
</xsl:template>

<lookup:singlefuncs>
    <singlefuncs name="SQRT" source="sqrt"/>
    <singlefuncs name="ROUND" source="round"/>
    <singlefuncs name="TRUNC" source="trunc"/>
    <singlefuncs name="CEIL" source="ceil"/>
    <singlefuncs name="FLOOR" source="floor"/>
    <singlefuncs name="ABS" source="abs"/>
    <singlefuncs name="STRLEN" source="strlen"/>
    <singlefuncs name="ArrayMaxSize" source="arrayMaxSize"/>
    <singlefuncs name="ArraySize" source="arraySize"/>
</lookup:singlefuncs>
<xsl:variable name="singlefuncs-lookup" select='document("")//lookup:singlefuncs/singlefuncs'/>

<xsl:template match="SQRT|ROUND|TRUNC|CEIL|FLOOR|ABS|STRLEN|ArrayMaxSize|ArraySize">
    <xsl:variable name="name" select="local-name(.)"/>
    <xsl:value-of select="$singlefuncs-lookup[@name = $name]/@source"/>
    <xsl:text>(</xsl:text>
    <xsl:apply-templates select="*" />
    <xsl:text>)</xsl:text>
</xsl:template>

<xsl:template match="NodeRef|NodeId">
    <xsl:value-of select="if (@dir = 'self') then 'Self' else replace(., '-', '_dash_')"></xsl:value-of>
</xsl:template>

<lookup:nodevar>
    <nodevar name="NodeOutcomeVariable" suffix=".outcome"/>
    <nodevar name="NodeStateVariable" suffix=".state"/>
    <nodevar name="NodeFailureVariable" suffix=".failure"/>
    <nodevar name="NodeCommandHandleVariable" suffix=".command_handle"/>
    <nodevar name="NodeTimepointValue" suffix=".of"/>
</lookup:nodevar>
<xsl:variable name="nodevar-lookup" select='document("")//lookup:nodevar/nodevar'/>

<xsl:template match="NodeOutcomeVariable|NodeStateVariable|NodeFailureVariable|NodeCommandHandleVariable">
    <xsl:apply-templates select="NodeRef|NodeId" />
    <xsl:variable name="name" select="local-name(.)"/>
    <xsl:value-of select="$nodevar-lookup[@name = $name]/@suffix"/>
</xsl:template>

<xsl:template match="NodeTimepointValue">
    <xsl:apply-templates select="NodeRef|NodeId" />
    <xsl:value-of select="concat('.', NodeStateValue, '.', Timepoint)"/>
</xsl:template>

<xsl:template match="*">
    <xsl:message terminate="no">
    WARNING: Unmatched element: <xsl:value-of select="name()"/>
    </xsl:message>
</xsl:template>

</xsl:transform>
