#
# C/C++ compiler flag overrides
#

DEFINES			+= -DDarwin -DPLATFORM_HAS_GNU_HASH_MAP
DEBUG_FLAGS		+= -gfull
OPENGL_EXTRA_FLAGS	:= -framework OpenGL -framework GLUT

ifeq (4,$(CXX_MAJOR_VERSION))
DEFINES		+= -DPLATFORM_HAS_EXECINFO_H -DPLATFORM_HAS_TR1_UNORDERED_MAP
endif

#
# Compiler/linker option overrides
#

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG			:= 
# Linker flag for run-time library search path
RUNTIME_SHARED_LIBRARY_PATH_FLAG	:=
# Linker flag to construct shared library
SHARED_FLAGS				:= -fno-common -dynamiclib
# Extension for shared library
SUFSHARE				:= .dylib
# Name of the library with the pthreads API
PTHREAD_LIB   	      	       		:= pthread
