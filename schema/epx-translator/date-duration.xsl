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

<!-- This stylesheet requires XSLT 2.0 for xsl:function, xsl:sequence -->
<!-- This stylesheet requires XPath 2.0 for 'except' -->

<xsl:transform version="2.0"
               xmlns:tr="extended-plexil-translator"
               xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
               xmlns:fn="http://www.w3.org/2005/xpath-functions"
               exclude-result-prefixes="fn tr xs">

  <!-- Dates and Durations (handled rather naively as real values) -->

  <xsl:template match="Type[.='Date' or .='Duration']">
    <!-- Dates and Durations are represented as "real" values -->
    <Type>Real</Type>
  </xsl:template>

  <xsl:template match="DurationVariable|DateVariable">
    <!-- Dates and Durations are represented as "real" variables -->
    <RealVariable><xsl:value-of select="."/></RealVariable>
  </xsl:template>

  <xsl:template match="DateValue">
    <!-- A Date is the number of seconds since the start of the epoch used on this platform -->
    <RealValue>
      <xsl:value-of select="tr:seconds(xs:dateTime(.) - xs:dateTime($epoch))"/>
    </RealValue>
  </xsl:template>

  <xsl:template match="DurationValue">
    <!-- A Duration is the number of seconds in the ISO 8601 duration -->
    <RealValue><xsl:value-of select="tr:seconds(.)"/></RealValue>
  </xsl:template>

  <!-- Some Epochs http://en.wikipedia.org/wiki/Epoch_(reference_date) -->

  <!-- Julian day 2415021 (DJD+1?) -->
  <xsl:variable name="UTC" select="xs:dateTime('1900-01-01T00:00:00Z')"/>

  <!-- Julian day 2440587.5 -->
  <xsl:variable name="Unix" select="xs:dateTime('1970-01-01T00:00:00Z')"/>

  <!-- Julian day 2448250 -->
  <xsl:variable name="CPS" select="xs:dateTime('1990-12-24T00:00:00Z')"/>

  <!-- Julian date 2451545.0 TT -->
  <xsl:variable name="J2000" select="xs:dateTime('2000-01-01T11:58:55.816Z')"/>

  <!-- Use this epoch -->
  <xsl:variable name="epoch" select="$Unix"/>

  <!-- Return the (total) number of seconds in and ISO 8601 duration -->
  <xsl:function name="tr:seconds">
    <xsl:param name="duration"/>
    <xsl:value-of select="xs:dayTimeDuration($duration) div xs:dayTimeDuration('PT1.0S')"/>
  </xsl:function>

</xsl:transform>
