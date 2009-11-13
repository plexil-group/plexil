# Standard macro definitions for Plexil make files.


##### Basic utilities and Unix commands

SHELL           = /bin/sh
RM		= /bin/rm -fr  # force, recursive
LS              = /bin/ls
DEPEND		= makedepend -f Makedepend
TAG		= etags -t
MV              = /bin/mv
MKDIR           = /bin/mkdir
CP              = /bin/cp -p

# Define when purify is available.  When it is desired, edit the file
# src/main/targets.make, as instructed there.
PURIFY		= purify


# KMD: Need to get right compiler flags...
CC	        = gcc

# KMD: Command for building archive libraries, if applicable.
AR		= $(CC) -ar 

##### C++ compiler options.

# KMD: Need proper flags for warning suppression...
# Note: we do not normally skip any warnings.  The exceptions for now are:
# 1355: extra trailing ';'
# 1009: A "/*" appears inside a comment.
# 3322: omission of explicit return type (pervasive in X11 files)
# 1356: nonstandard member constant declaration
# 1401: qualified name in class member declaration
# 1375: base class destructor not virtual
# 1201: trailing comma
# 1234: "access control not specified (public by default)" -- found in some STL
# SKIP_WARNINGS = -woff 1355,1009,3322,1356,1375,1401,1201,1234

# KMD: determine compiler flags we need...
# STANDARD_CFLAGS		= -multigot $(SKIP_WARNINGS)

# There are three kinds of builds: debuggable, profilable,
# and optimized.  These are achieved through using the following options for CC

# This is used in standard day to day builds.
CC_DEBUG        = $(CC) -gfull -g3 $(STANDARD_CFLAGS)

# KMD: determine profiling build, if applicable
# CC_PROFILE      = insight -ptused

# KMD: determine optimized build arguments
# CC_OPTIMIZE     = $(CC) +w -O3 $(STANDARD_CFLAGS)


##### Library support

# Default name for library.  Must be redefined to be useful.
LIBRARY        = dummy


##### Conveniences

# Default for CVS targets in svn.make; should be shadowed to be more useful.
SVN_FILES       = *

##### Suffix rules

# KMD: determine if useful
# .SUFFIXES : .cc .hh .o .c .h
