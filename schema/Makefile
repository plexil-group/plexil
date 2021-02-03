# Copyright (c) 2006-2021, Universities Space Research Association (USRA).
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Universities Space Research Association nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# This makefile generates RELAX NG Compact schemas from XML Schemas.
# A *.rnc RELAX-NG compact schema file will be generated for each
# *.xsd file found in this Makefile's directory.
#
# To generate RELAX-NG Compact schema, type:
# $ make
#
# To remove all generated RELAX-NG Compact schema, type:
# $ make clean
#
# For information about RELAX NG, including information about the
# conversion tools used by this makefile, see http://relaxng.org/.
#
# This makefile requires the following tools:
#
# XSDtoRNG.xsl - XSL transformation script to transform a schema
# into RELAX NG form. See https://github.com/chucko58/XSDtoRNG .
# Licensed under Apache License 2.0.
#
# Saxon 9 Home Edition (saxon9he) - XSL processor, free edition,
# from http://saxonica.com/ .
# 
# trang - Trang converts, among other things, a RELAX NG schema to
# RELAX NG and RELAX NG Compact form. As of 2020, get the latest
# release as a zipped jar file at:
# https://github.com/relaxng/jing-trang/releases
#
# To summarize what this makefile does: 
#
# Given a PlexilPlan XML Schema called PLEXIL_schema.xsd,
# to convert it to RELAX NG Compact format:
#
# $ java -jar ../jars/saxon9he.jar -xsl:XSDtoRNG.xsl -s:PLEXIL_schema.xsd  -o:PLEXIL_schema.rng
# $ java -jar ../jars/trang.jar -I rng -O rnc PLEXIL_schema.rng PLEXIL_schema.rnc
#
#------------------------------------------------------------------------------

include ../makeinclude/standard-defs.make

SCHEMA_XSD = plexil-base.xsd core-plexil.xsd safe-plexil.xsd extended-plexil.xsd
SCHEMA_RNC = $(SCHEMA_XSD:.xsd=.rnc)

PLEXILSCRIPT_XSD = plexil-script.xsd
PLEXILSCRIPT_RNC = $(PLEXILSCRIPT_XSD:.xsd=.rnc)

SAXON_JAR = ../jars/saxon9he.jar

# Most recent version is 20181222
TRANG_JAR = ../jars/trang.jar

plexil-default: $(SCHEMA_RNC) $(PLEXILSCRIPT_RNC)

plexil-script: plexil-script.rnc

dust:
	@$(RM) -f *.rng

clean: dust
	@$(RM) -f $(SCHEMA_RNC) $(PLEXILSCRIPT_RNC)

# Schema validator cache must be invalidated if any PLEXIL schema changes
invalidate-validator-cache:
	@$(RM) -rf validator/schema_cache

.PHONY: invalidate-validator-cache

# Dependencies
extended-plexil.rnc core-plexil.rnc : plexil-base.rng

$(SCHEMA_XSD:.xsd=.rng): %.rng: %.xsd | invalidate-validator-cache
	$(JAVA) -jar $(SAXON_JAR) -xsl:XSDtoRNG.xsl -s:$< -o:$@ start=PlexilPlan

# Special rule for Plexilscript only
plexil-script.rng: plexil-script.xsd
	$(JAVA) -jar $(SAXON_JAR) -xsl:XSDtoRNG.xsl -s:$< -o:$@ start=PLEXILScript

# General rule to create a RELAX NG schema from an XML schema.
# %.rng: %.xsd
# 	$(JAVA) -jar $(SAXON_JAR) -xsl:XSDtoRNG.xsl -s:$< -o:$@

# The rule creates a RELAX NG compact schema from a RELAX NG schema.
%.rnc: %.rng
	$(JAVA) -jar $(TRANG_JAR) -I rng -O rnc $< $@

# Uncomment these if you want to see these intermediate files.
#.PRECIOUS: %.rng
