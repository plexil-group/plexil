#
# C/C++ compiler flag overrides
#

DEFINES		+= -DPLATFORM_HAS_EXECINFO_H -DPLATFORM_HAS_GNU_HASH_MAP

ifeq (4,$(CXX_MAJOR_VERSION))
# TR1_UNORDERED_MAP is broken in g++ 4.0.x
ifneq (0,$(CXX_MINOR_VERSION))
DEFINES		+= -DPLATFORM_HAS_TR1_UNORDERED_MAP
endif
endif

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
# Name of system library with dynamic loading API
DLOPEN_LIB				:= dl
# Name of the library with the pthreads API
PTHREAD_LIB   	      	       	        := pthread
# Name of system library with realtime clock API
RT_LIB					:= rt
