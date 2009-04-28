SRC = ThreadSpawn

CXX = g++

OSTYPE = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
 ifeq ($(findstring DARWIN, $(OSTYPE)), DARWIN)
  LIB_EXT = .dylib
  SHARED_LIB_FLAGS += -dynamiclib -Wl
  SHARED_OBJ_FLAGS += -fPIC
 else
  LIB_EXT = .so
  SHARED_LIB_FLAGS += -z combreloc -shared -Wl
  SHARED_OBJ_FLAGS += -fpic
 endif

% :
	$(CXX) $(INCLUDES) -fPIC $@.cc -c
	ar cru liblcmUtils.a $@.o

all: $(SRC)
	$(CXX) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o liblcmUtils.so *.o
	ranlib liblcmUtils.a
	(cd structs; make clean; make all)

default: all


clean:
	(cd structs; make clean)
	rm *.so *.a *.o