STRUCTS = driveCommand driveResponse telemetryDouble

CXX = g++

ifndef LCM_PATH
	LCM_PATH = $(PLEXIL_HOME)/interfaces/lcm-0.2.1
endif

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

LCMGEN := $(LCM_PATH)/bin/lcm-gen
INCLUDES := -I$(LCM_PATH)
OUTPUT_DIR := generatedFiles

make-dirs:
	mkdir -p include
	mkdir -p lib
	mkdir -p $(OUTPUT_DIR)

% :
	$(LCMGEN) $@.lcm -c
	$(CXX) $(INCLUDES) -fPIC $@.c -c
	ar cru libComStructs.a $@.o

all: make-dirs $(STRUCTS)
	$(CXX) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o libComStructs.so *.o -L$(LCM_PATH)/lib -llcm
	mv *.so lib
	mv *.o $(OUTPUT_DIR)
	mv *.c $(OUTPUT_DIR)
	mv *.h include
	ranlib libComStructs.a
	mv *.a lib

default: all


clean:
	rm -rf include
	rm -rf lib
	rm -rf $(OUTPUT_DIR)