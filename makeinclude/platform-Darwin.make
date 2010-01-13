#
# C/C++ compiler flag overrides
#

DEFINES		+= -DDarwin
DEBUG_FLAGS	+= -gfull
OPENGL_LIBS	:= -framework OpenGL -framework GLUT

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
