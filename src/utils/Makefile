
include ../../makeinclude/standard-defs.make

LIBRARY		= utils

# External dependencies
INC_DIRS	+= ../../third-party/tinyxml
LIB_PATH	+= ../../third-party/tinyxml
LIBS		+= tinyxml

SRC = $(wildcard *.cc)
INC = $(SRC:.cc=.hh) CommonDefs.hh Id.hh TestSupport.hh

SVN_FILES       = Makefile $(SRC) $(INC)

include ../../makeinclude/standard-targets.make
-include Makedepend
