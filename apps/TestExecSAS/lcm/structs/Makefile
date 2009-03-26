STRUCTS = driveCommand driveResponse telemetryDouble

CXX = g++

ifndef LCM_PATH
	LCM_PATH = /home/bvk/Packages/lcm-0.2.1
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
	$(CXX) -fPIC -shared -o libComStructs.so *.o
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