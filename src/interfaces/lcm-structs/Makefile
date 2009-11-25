
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

LIBRARY	= ComStructs

STRUCTS := $(wildcard *.lcm)
STRUCTS_C := $(STRUCTS:%.lcm=%.c)
STRUCTS_H := $(STRUCTS:%.lcm=%.h)
STRUCTS_O := $(STRUCTS:%.lcm=%.o)

SRC = $(STRUCTS_C)
INC =

SVN_FILES	= Makefile $(STRUCTS)

LIBS = lcm

DIRT += $(STRUCTS_C)

ifndef LCM_PATH
LCM_PATH = $(PLEXIL_HOME)/third-party/lcm
endif
LCMGEN := $(PLEXIL_HOME)/bin/lcm-gen
INCLUDES := -I$(LCM_PATH)

all: default

# Pattern rule for generating C source code from LCM
%.c %.h : %.lcm 
	$(LCMGEN) $< -c

include $(PLEXIL_HOME)/makeinclude/standard-targets.make
-include Makedepend
