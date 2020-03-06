# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

# The original version of this Makefile was contributed by Jeff Kowing
# (jeffrey.d.kowing@nasa.gov)
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
# trang - Trang converts, among other things, a RELAX NG schema to
# RELAX NG and RELAX NG Compact form.  As of 2020, get the latest
# release as a zipped jar file at:
# https://github.com/relaxng/jing-trang/releases
#
# rncfix - A bash script for fixing up the generated PlexilPlan RELAX
# NG Compact schema.
#
# To summarize what this makefile does: 
#
# Given a PlexilPlan XML Schema called PLEXIL_schema.xsd,
# to convert it to RELAX NG Compact format:
#
# $ java -jar trang.jar -I xml -O rnc PLEXIL_schema.rng PLEXIL_schema.rnc.in
# 
# PLEXIL_schema.rnc.in is the RELAX NG compact form.  But a few fixes
# have to be made, so the final step is:
#
# $ rncfix PLEXIL_schema.rnc.in PLEXIL_schema.rnc 
#
#------------------------------------------------------------------------------

include $(PLEXIL_HOME)/makeinclude/standard-defs.make

# A script to fix up the generated RELAX-NG Compact schema.
RNCFIX := ./rncfix

SCHEMA_V1_XSD = core-plexil.xsd safe-plexil.xsd extended-plexil.xsd plexil-script.xsd
SCHEMA_V1_RNC    = $(SCHEMA_V1_XSD:.xsd=.rnc)
SCHEMA_V1_RNC_IN = $(SCHEMA_V1_XSD:.xsd=.rnc.in)

SCHEMA_V2_XSD = core-plexil-v2.xsd extended-plexil-v2.xsd
SCHEMA_V2_RNC =    $(SCHEMA_V2_XSD:.xsd=.rnc)
SCHEMA_V2_RNC_IN = $(SCHEMA_V2_XSD:.xsd=.rnc.in)

# Most recent version is 20181222
TRANG_JAR = ../jars/trang.jar

plexil-default: schema-v1 schema-v2 plexil-script

schema-v1: schema-v1-rnc
schema-v1-rnc: $(SCHEMA_V1_XSD:.xsd=.rnc)

schema-v2: schema-v2-rnc
schema-v2-rnc: $(SCHEMA_V2_XSD:.xsd=.rnc)

plexil-script: plexil-script.rnc

# The .rng files, if any, are an artifact of a previous version
# of this makefile.
dust:
	@$(RM) -f *.rnc.in *.rng

clean: dust
	@$(RM) -f $(SCHEMA_V1_RNC) $(SCHEMA_V2_RNC) plexil-script.rnc

$(SCHEMA_V1_RNC_IN) : plexil-base.xsd

$(SCHEMA_V2_RNC_IN) : plexil-base-v2.xsd

# The rule creates a RELAX-NG compact schema from an XML schema.
%.rnc.in: %.xsd
	$(JAVA) -jar $(TRANG_JAR) -I xml -O rnc $< $@

# The rule makes some fixes to the generated RELAX-NG compact schema.
%.rnc: %.rnc.in
	$(RNCFIX) -f $< $@

# Uncomment these if you want to see these intermediate files.
#.PRECIOUS: %.rnc.in 
