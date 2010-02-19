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
INC_DIRS	+= $(WIND_BASE)/target/usr/h $(WIND_BASE)/target/h/wrn/coreip

# Define this as a Real Time Process project

DEFINES		+= -DPLEXIL_VXWORKS -D__RTP__ -DCPU=$(CPU) -DTOOL=$(TOOL) -DTOOL_FAMILY=$(TOOL_FAMILY) \
                   -DPLATFORM_HAS_DINKUM_HASH_MAP

STANDARD_CFLAGS		+= $(CC_ARCH_SPEC)
STANDARD_CXXFLAGS	+= $(CC_ARCH_SPEC)

ifneq ($(PLEXIL_OPTIMIZE),)
VARIANT_CFLAGS		+= $(CC_OPTIM_NORMAL)
endif

# Kludge around some things the Wind River Workbench includes but the standard make includes don't
ifeq ($(CPU),PPC604)
CPU_FAMILY := PPC32
endif

ifeq ($(CPU),PPC32)
CPU_FAMILY := PPC32
endif

ifeq ($(CPU_FAMILY),PPC32)
STANDARD_CFLAGS		+= -mhard-float -mstrict-align -mregnames -ansi -mrtp -Wall  -MD -MP
STANDARD_CXXFLAGS	+= -mhard-float -mstrict-align -mregnames -ansi -mrtp -Wall  -MD -MP

# Compiler flags for shared libraries
POSITION_INDEPENDENT_CODE_FLAG	:= -fpic
endif

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
PTHREAD_LIB   	      	       		:= pthreadLib
