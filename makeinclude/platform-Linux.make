#
# C/C++ compiler flag overrides
#

DEFINES		+= -DPLATFORM_HAS_EXECINFO_H -DPLATFORM_HAS_GNU_HASH_MAP

ifeq (4,$(CXX_MAJOR_VERSION))
DEFINES		+= -DPLATFORM_HAS_TR1_UNORDERED_MAP
endif

OPENGL_LIB_PATH := /usr/X11R6/lib
OPENGL_LIBS	:= GL GLU glut Xi Xmu

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
