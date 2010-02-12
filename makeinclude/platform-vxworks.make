# Platform definitions for cross-compilation for VxWorks RTOS

ifndef WIND_BASE
$(error Required environment variable WIND_BASE is not set. Exiting.)
endif

ifndef CPU
$(error Required environment variable CPU is not set. Exiting.)
endif

# Default to the Gnu toolchain.
TOOL_FAMILY ?= gnu
TOOL ?= gnu

# work around an ordering bug in defs.library
TGT_DIR=$(WIND_BASE)/target
include $(WIND_BASE)/target/h/make/defs.library

##
## Plexil-specific customizations
##

#
# C/C++ compiler flag overrides
#

# Header file path for targets
SYSTEM_INC_DIRS	+= $(WIND_BASE)/target/usr/h $(WIND_BASE)/target/usr/h/c++

# Define this as a Real Time Process project
STANDARD_CFLAGS		+= -D__RTP__
STANDARD_CXXFLAGS	+= -D__RTP__

OPENGL_LIBS	:= -lGL -lGLU -lglut

#
# Compiler/linker option overrides
#

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG			:= -Wl,
# Linker flag for run-time library search path
RUNTIME_SHARED_LIBRARY_PATH_FLAG	:= -rpath
# Linker flag to construct shared library
SHARED_FLAGS				:= -shared
# Extension for shared library
SUFSHARE				:= .so
# Name of the library with the pthreads API
PTHREAD_LIB   	      	       		:= pthread
