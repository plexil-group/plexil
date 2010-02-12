#
# C/C++ compiler flag overrides
#

DEFINES			+= -DDarwin -DPLATFORM_HAS_GNU_HASH_MAP
DEBUG_FLAGS		+= -gfull
OPENGL_EXTRA_FLAGS	:= -framework OpenGL -framework GLUT

# *** FIXME: This is only true of Mac OS X 10.4.x and newer
DEFINES		+= -DPLATFORM_HAS_EXECINFO_H

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
