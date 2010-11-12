+ #
# C/C++ compiler flag overrides
#

DEFINES		+= -DPLATFORM_HAS_GNU_HASH_MAP

OPENGL_LIBS	:= GL GLU glut

#
# Compiler/linker option overrides
#

# Compiler flag to pass an argument to the linker
LINKER_PASSTHROUGH_FLAG			:= -Wl,
# Linker flag for run-time library search path
RUNTIME_SHARED_LIBRARY_PATH_FLAG	:= -rpath
# Linker flag to construct shared library
# Note that '=' (and not ':=') is vital, since we want $(LIBRARY) to be expanded
# All the extra stuff is to make Cygwin gcc generate DLL import libraries.
SHARED_FLAGS				= -shared -Wl,--out-implib=lib$(LIBRARY).dll.a \
                  -Wl,--export-all-symbols \
                  -Wl,--enable-auto-import \
                  -Wl,--no-whole-archive
# -Wl,--whole-archive $old_lib \

# Extension for shared library
SUFSHARE				:= .dll
# Name of the library with the pthreads API
PTHREAD_LIB   	      	       		:= pthread
