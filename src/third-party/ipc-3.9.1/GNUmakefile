#############################################################################
#       $Id: GNUmakefile,v 2.7 2011/08/17 01:15:23 reids Exp $
# $Revision: 2.7 $
#     $Date: 2011/08/17 01:15:23 $
#   $Author: reids $
#    $State: Exp $
#   $Locker:  $
#
# PROJECT:	IPC
#
# FILE:		GNUmakefile
#		Based on Erebus prototypical Makefile
#		Makefile.proto,v 1.1 1992/06/16 22:50:21 jwest Exp 
#
# DESCRIPTION:
# Makes the ipc libraries and central server, Lisp version and test program
#
# EXPORTS:
#	The following targets are available as part of Makefile.std:
#		all bins libs privs objs
#		clean install
#		lint depend
#
# Copyright (c) 2008, Carnegie Mellon University
#     This software is distributed under the terms of the 
#     Simplified BSD License (see ipc/LICENSE.TXT)
#
# HISTORY:
#
# $Log: GNUmakefile,v $
# Revision 2.7  2011/08/17 01:15:23  reids
# Added the python directory to the makefile
#
# Revision 2.6  2009/02/27 15:06:41  reids
# Added LICENSE.TXT to ftp distribution
#
# Revision 2.5  2009/01/12 15:54:54  reids
# Added BSD Open Source license info
#
# Revision 2.4  2002/06/25 21:20:56  reids
# Don't make privs when doing gmake install
#
# Revision 2.3  2002/01/02 21:40:08  reids
# Changed makefile so that java directory can be installed from top level.
#
# Revision 2.2  2001/06/01 18:49:37  reids
# Make xdrgen from top level
#
# Revision 2.1.1.1  1999/11/23 19:07:32  reids
# Putting IPC Version 2.9.0 under local (CMU) CVS control.
#
#############################################################################

INSTALL_DIR	= .
ifndef BASE_DIR
BASE_DIR	:= $(shell cd $(INSTALL_DIR);/bin/pwd)
endif
MODULE_DIR	= 
PROJECT		= ipc
MODULE		= 
SUBDIRS		= etc src lisp java python test doc xdrgen

#############################################################################
# Where are the standard files?
#############################################################################

ifndef FAST_COMPILE
PWD 	:= $(shell /bin/pwd)
endif

ifndef STD_FILE
STD_FILE := $(shell ls $(BASE_DIR)/etc/GNUmakefile.std | grep -v "not found")
ifeq ($(STD_FILE),)
STD_FILE := $(PROJECT_DIR)/etc/GNUmakefile.std
endif
DEFS_FILE := $(shell ls $(BASE_DIR)/etc/GNUmakefile.defs | grep -v "not found")
ifeq ($(DEFS_FILE),)
DEFS_FILE := $(PROJECT_DIR)/etc/GNUmakefile.defs
endif
endif

##########
# Include standard definitions
##########

include $(DEFS_FILE)

##########
# Override defaults here.
##########

##########
# File Definition Macros.  Deletion of generated files assumed
#    to be o.k.  Source files will never be deleted.
##########

# Generated files - installed
PUBLIC_BINS		= 
PUBLIC_LIBS		= 
PUBLIC_LINTS		= 

MODULE_LIBS		= 
MODULE_LINTS		= 

# Source files - installed
PUBLIC_INCS		= 
PUBLIC_MANS		= 
PUBLIC_DOCS		= 

MODULE_INCS		= 
MODULE_DOCS		= 

INCS			= $(MODULE_INCS) $(PUBLIC_INCS)

# Generated files - not installed
PRIVS	= 

OBJS	= 

# Source files - not installed

SRCS	= 

PRIVATE_INCS	= 

MISC		=  GNUmakefile README README.MAC LICENSE.TXT

##########
# Default Target
##########

default: install

all:: srcs libs bins privs

##########
# User Defined Targets
##########

# Define rules for all files listed in BINS, LIBS, and LINTS in this section.
# Example executable, object, library, and combined objects are shown below:
#
#  $(BDIR)/sample: file1.o file2.o $(LIBDEP)
#	$(LINK.c) -o $@ file1.o file2.o $(LIBRARIES)
#
#  $(ODIR)/alternateName.o: file1.c
#	$(COMPILE.c) -o $@ file1.c
#
#  $(LDIR)/libsample.a: file1.o file2.o
#	-$(RM) $@
#	$(AR) $(ARFLAGS) $@ file1.o file2.o
#	$(RANLIB) $@
#
#  $(ODIR)/combined.o: $(ODIR)/part1.o $(ODIR)/part2.o
#	$(LD) -r -o $@ $(ODIR)/part1.o $(ODIR)/part2.o; chmod a-x $@

##########
# Include standard target definitions and RCS rules
##########

include $(STD_FILE)
