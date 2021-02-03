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

  <!-- SynchronousCommand -->

  <!-- FIXME: Simplify -->

  <xsl:template match="SynchronousCommand">
    <Node NodeType="NodeList" epx="SynchronousCommand">
      <xsl:call-template name="copy-source-locator-attributes" />
      <xsl:call-template name="ensure-unique-node-id" />
      <xsl:call-template name="standard-preamble" />
      <NodeBody>
        <NodeList>
          <xsl:choose>
            <xsl:when test="Command/*[fn:ends-with(fn:name(), 'Variable')]">
              <xsl:call-template name="command-with-return" />
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name="command-without-return" />
            </xsl:otherwise>
          </xsl:choose>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

  <xsl:template name="command-with-return">
    <Node NodeType="NodeList" epx="aux">
      <NodeId generated="1">
        <xsl:value-of select="tr:prefix('SynchronousCommandAux')" />
      </NodeId>
      <xsl:choose>
        <xsl:when test="Command/ArrayVariable">
          <xsl:call-template name="command-with-array-return-body" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="command-with-scalar-return-body" />
        </xsl:otherwise>
      </xsl:choose>
    </Node>
  </xsl:template>

  <xsl:template name="command-with-scalar-return-body">
    <xsl:variable name="return" select="tr:prefix('return')"/>
    <xsl:variable name="orig-ref"
                  select="Command/(BooleanVariable|IntegerVariable|RealVariable|StringVariable)" />
    <xsl:variable name="var-type" select="fn:substring-before(name($orig-ref), 'Variable')" />
    <xsl:variable name="ref">
      <xsl:element name="{name($orig-ref)}">
        <xsl:value-of select="$return"/>
      </xsl:element>
    </xsl:variable>

    <VariableDeclarations>
      <xsl:call-template name="declare-variable">
        <xsl:with-param name="name" select="$return"/>
        <xsl:with-param name="type" select="$var-type"/>
      </xsl:call-template>
    </VariableDeclarations>
    <xsl:if test="Timeout">
      <InvariantCondition>
        <xsl:call-template name="timed-out">
          <xsl:with-param name="element" select="Timeout/*"/>
        </xsl:call-template>
      </InvariantCondition>
    </xsl:if>
    <NodeBody>
      <NodeList>
        <Node NodeType="Command" epx="aux">
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
          </NodeId>
          <EndCondition>
            <IsKnown><xsl:sequence select="$ref"/></IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <xsl:sequence select="Command/ResourceList"/>
              <xsl:sequence select="$ref"/>
              <xsl:sequence select="Command/Name"/>
              <xsl:sequence select="Command/Arguments"/>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment" epx="aux">
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandAssignment')" />
          </NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">
                <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
              </NodeRef>
            </Finished>
          </StartCondition>
          <NodeBody>
            <Assignment>
              <xsl:sequence select="$orig-ref"/>
              <xsl:choose>
                <xsl:when test="Command/BooleanVariable">
                  <BooleanRHS><xsl:sequence select="$ref"/></BooleanRHS>
                </xsl:when>
                <xsl:when test="Command/StringVariable">
                  <StringRHS><xsl:sequence select="$ref"/></StringRHS>
                </xsl:when>
                <xsl:otherwise>
                  <NumericRHS><xsl:sequence select="$ref"/></NumericRHS>
                </xsl:otherwise>
              </xsl:choose>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </xsl:template>

  <xsl:template name="command-with-array-return-body">
    <xsl:variable name="return" select="tr:prefix('return')"/>
    <xsl:variable name="array_name" select="Command/ArrayVariable"/>
    <xsl:variable name="ref">
      <ArrayVariable> <xsl:value-of select="$return"/> </ArrayVariable>
    </xsl:variable>

    <VariableDeclarations>
      <DeclareArray>
        <Name><xsl:value-of select="$return"/></Name>
        <!-- A royal hack!  Couldn't find a more compact expression that worked. -->
        <xsl:choose>
          <!-- First see if the array we are proxying is local -->
          <xsl:when test="VariableDeclarations/DeclareArray[Name = $array_name][last()]">
            <xsl:sequence select="VariableDeclarations/DeclareArray[Name = $array_name][last()]/Type"/>
            <xsl:sequence select="VariableDeclarations/DeclareArray[Name = $array_name][last()]/MaxSize"/>
            <!-- Otherwise find it in the closest ancestor -->
          </xsl:when>
          <xsl:when test="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]">
            <xsl:sequence select="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]/Type"/>
            <xsl:sequence select="ancestor::*/VariableDeclarations/DeclareArray[Name = $array_name][last()]/MaxSize"/>
          </xsl:when>
        </xsl:choose>
      </DeclareArray>
    </VariableDeclarations>
    <xsl:if test="Timeout">
      <InvariantCondition>
        <xsl:call-template name="timed-out">
          <xsl:with-param name="element" select="Timeout/*"/>
        </xsl:call-template>
      </InvariantCondition>
    </xsl:if>
    <NodeBody>
      <NodeList>
        <Node NodeType="Command" epx="aux">
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
          </NodeId>
          <EndCondition>
            <IsKnown>
              <ArrayElement>
                <xsl:sequence select="$ref"/>
                <Index><IntegerValue>0</IntegerValue></Index>
              </ArrayElement>
            </IsKnown>
          </EndCondition>
          <NodeBody>
            <Command>
              <xsl:sequence select="Command/ResourceList"/>
              <xsl:sequence select="$ref"/>
              <xsl:sequence select="Command/Name"/>
              <xsl:sequence select="Command/Arguments"/>
            </Command>
          </NodeBody>
        </Node>
        <Node NodeType="Assignment" epx="aux">
          <NodeId generated="1">
            <xsl:value-of select="tr:prefix('SynchronousCommandAssignment')" />
          </NodeId>
          <StartCondition>
            <Finished>
              <NodeRef dir="sibling">
                <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
              </NodeRef>
            </Finished>
          </StartCondition>
          <NodeBody>
            <Assignment>
              <xsl:sequence select="$array_name"/>
              <ArrayRHS><xsl:sequence select="$ref"/></ArrayRHS>
            </Assignment>
          </NodeBody>
        </Node>
      </NodeList>
    </NodeBody>
  </xsl:template>
  
  <xsl:template name="command-without-return">
    <Node NodeType="NodeList" epx="aux">
      <NodeId generated="1">
        <xsl:value-of select="tr:prefix('SynchronousCommandAux')" />
      </NodeId>
      <xsl:if test="Timeout">
        <InvariantCondition>
          <xsl:call-template name="timed-out">
            <xsl:with-param name="element" select="Timeout/*"/>
          </xsl:call-template>
        </InvariantCondition>
      </xsl:if>
      <NodeBody>
        <NodeList>
          <Node NodeType="Command" epx="aux">
            <NodeId generated="1">
              <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
            </NodeId>
            <EndCondition>
              <OR>
                <EQInternal>
                  <NodeCommandHandleVariable>
                    <NodeId>
                      <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                    </NodeId>
                  </NodeCommandHandleVariable>
                  <NodeCommandHandleValue>COMMAND_SUCCESS</NodeCommandHandleValue>
                </EQInternal>
                <EQInternal>
                  <NodeCommandHandleVariable>
                    <NodeId>
                      <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                    </NodeId>
                  </NodeCommandHandleVariable>
                  <NodeCommandHandleValue>COMMAND_FAILED</NodeCommandHandleValue>
                </EQInternal>
                <EQInternal>
                  <NodeCommandHandleVariable>
                    <NodeId>
                      <xsl:value-of select="tr:prefix('SynchronousCommandCommand')" />
                    </NodeId>
                  </NodeCommandHandleVariable>
                  <NodeCommandHandleValue>COMMAND_DENIED</NodeCommandHandleValue>
                </EQInternal>
              </OR>
            </EndCondition>
            <NodeBody>
              <Command>
                <xsl:sequence select="Command/ResourceList"/>
                <xsl:sequence select="Command/Name"/>
                <xsl:sequence select="Command/Arguments"/>
              </Command>
            </NodeBody>
          </Node>
        </NodeList>
      </NodeBody>
    </Node>
  </xsl:template>

</xsl:transform>

