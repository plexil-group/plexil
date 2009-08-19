# This Makefile was originally created by Jeff Kowing
# (jeffrey.d.kowing@nasa.gov)
#
# Modifications:
#
#   - requires environment variable RNGCONV_JAR to specify the rngconv
#     application jar file, and TRANG_JAR to specify the Trang
#     application jar file.  See below for more information on these
#     applications.  (Mike Dalal)
#   - adds the 'make format' target that formats plexil.xsd into
#     plexil-formatted.xsd (Mike Dalal)
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
# rngconv - Sun RELAX NX Converter.  This software converts, among
# other things, an XML Schema to a RELAX NG schema.  After
# registering, you can get it at
# http://wwws.sun.com/software/xml/developers/relaxngconverter/.
#
# trang - Trang converts, among other things, a RELAX NG schema to
# RELAX NG Compact form.  Get it at
# http://www.thaiopensource.com/download/trang-20030619.zip.
#
# rncfix - My simple bash script for fixing up the generated
# PlexilPlan RELAX NG Compact schema.
#
# To summarize what this makefile does: 
#
# Given a PlexilPlan XML Schema called PLEXIL_schema.xsd, the steps to
# convert are as follows:
#
# $ java -jar rngconv.jar PLEXIL_schema.xsd > PLEXIL_schema.rng
#
# PLEXIL_schema.rng is the RELAX NG form.
#
# $ java -jar trang.jar PLEXIL_schema.rng PLEXIL_schema.rnc
# 
# PLEXIL_schema.rnc is the RELAX NG compact form.  But a few fixes
# have to be made, so the final step is:
#
# $ mv PLEXIL_schema.rnc PLEXIL_schema.rnc.in
# $ rncfix PLEXIL_schema.rnc.in PLEXIL_schema.rnc 
#
#------------------------------------------------------------------------------

# My bone head script to fix up the generated RELAX-NG Compact schema.
RNCFIX := ./rncfix

SCHEMA_XSD = core-plexil.xsd safe-plexil.xsd extended-plexil.xsd \
	     core-plexil-draft.xsd
SCHEMA_RNG =    $(SCHEMA_XSD:.xsd=.rng)
SCHEMA_RNC =    $(SCHEMA_XSD:.xsd=.rnc)
SCHEMA_RNC_IN = $(SCHEMA_XSD:.xsd=.rnc.in)

default: all

all: $(SCHEMA_RNC) 

dust:
	@ -rm -f $(SCHEMA_RNG)
	@ -rm -f $(SCHEMA_RNC_IN)

clean: dust
	@ -rm -f $(SCHEMA_RNC)
	@ -rm -f plexil-formatted.xsd

# The rule creates a RELAX-NG schema from a XML Schema.
%.rng: %.xsd plexil-base.xsd
	java -jar $(RNGCONV_JAR) $< > $@

# The rule creates a RELAX-NG compact schema from a RELAX-NG schema.
%.rnc.in: %.rng
	java -jar $(TRANG_JAR) $< $@.rnc
	mv $@.rnc $@

# The rule makes some fixes to the generated RELAX-NG compact schema.
%.rnc: %.rnc.in
	$(RNCFIX) -f $< $@

# Uncomment these if you want to see these intermediate files.
#.PRECIOUS: %.rnc.in 
#.PRECIOUS: %.rng
