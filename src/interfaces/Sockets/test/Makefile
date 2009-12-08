
include $(PLEXIL_HOME)/makeinclude/standard-defs.make

EXECUTABLE	:= simple_client_main simple_server_main

# External dependencies
INC_DIRS	+= ..
LIB_PATH	+=
LIBS		+= tinyxml

SRC		:= $(wildcard *.cc)

SVN_FILES       := Makefile $(SRC)

# dummy for now
all: $(EXECUTABLE)

include $(PLEXIL_HOME)/makeinclude/standard-targets.make

# these must follow the above 'include'
simple_client_main: simple_client_main.o depend
	$(LD) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $@ $<

simple_server_main: simple_server_main.o depend
	$(LD) $(LIB_PATH_FLAGS) $(LIB_FLAGS) -o $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include Makedepend
endif
