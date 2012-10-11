#
# C/C++ compiler flag overrides
#

DEFINES			+= -DDarwin -DPLATFORM_HAS_GNU_HASH_MAP
DEBUG_FLAGS		+= -gfull

ifeq (4,$(CXX_MAJOR_VERSION))
DEFINES		+= -DPLATFORM_HAS_EXECINFO_H
# TR1_UNORDERED_MAP is broken in g++ 4.0.x
ifneq (0,$(CXX_MINOR_VERSION))
DEFINES		+= -DPLATFORM_HAS_TR1_UNORDERED_MAP
endif
endif

#
# Compiler/linker option overrides
#

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG				:= 
# Linker flag for run-time library search path
RUNTIME_SHARED_LIBRARY_PATH_FLAG	:=
# Linker flag to construct shared library
SHARED_FLAGS						:= -fno-common -dynamiclib
# Extension for shared library
SUFSHARE							:= .dylib
# Name of system library with dynamic loading API
DLOPEN_LIB							:= dl
# Name of the library with the pthreads API
PTHREAD_LIB 		   	       		:= pthread
# Name of system library with realtime clock API
# Is in libc on OS X
RT_LIB								:= 
