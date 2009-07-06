ifndef STANDALONE_SIMULATOR_HOME
	STANDALONE_SIMULATOR_HOME = $(PLEXIL_HOME)/apps/StandAloneSimulator
endif

SRC_DIR   = .

LIBS = -lpthread

LIBS += -L$(STANDALONE_SIMULATOR_HOME)/lib -lstandalonesimulator \
         -L$(PLEXIL_HOME)/lib/ \
	 -L$(PLEXIL_HOME)/interfaces/lcm-0.2.1/lib -llcm \
	 -L$(PLEXIL_HOME)/apps/TestExecSAS/lcm/structs/lib -lComStructs \
	 -L$(PLEXIL_HOME)/apps/TestExecSAS/lcm/ -llcmUtils

BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
INCLUDES = -I$(STANDALONE_SIMULATOR_HOME) -I$(PLEXIL_HOME)/apps/TestExecSAS/lcm/structs/include \
	   -I$(PLEXIL_HOME)/interfaces/lcm-0.2.1/include -I$(PLEXIL_HOME)/apps/TestExecSAS/lcm 

ifndef LCM_PATH
	LCM_PATH = $(PLEXIL_HOME)/interfaces/lcm-0.2.1
endif

 LIB_EXT = .so

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

PLEXILSIMULATOR_LIBS += $(LIB_DIR)/$(LIB_TARGET)

LIB_NAME = plexilsimulator

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
PLEXILSIMULATOR_TARGET = plexilSimulator

LIB_OBJECTS = $(OBJ_DIR)/PlexilSimResponseFactory$(OBJ_EXT) \
	      $(OBJ_DIR)/PlexilCommRelay$(OBJ_EXT)

PLEXILSIMULATOR_OBJECTS = $(OBJ_DIR)/PlexilSimulator$(OBJ_EXT)


############################
# Rules to build the targets

all : $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(PLEXILSIMULATOR_TARGET)

$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS) $(LIBS)

$(BIN_DIR)/$(PLEXILSIMULATOR_TARGET) : $(BIN_DIR) $(PLEXILSIMULATOR_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(PLEXILSIMULATOR_OBJECTS) $(PLEXILSIMULATOR_LIBS) $(LIBS)


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_OBJ_FLAGS) -c $< -o $@

#########################################
# Rules that make the directory structure

$(OBJ_DIR) :
	-mkdir -p $@

$(BIN_DIR) :
	-mkdir -p $@

$(LIB_DIR) :
	-mkdir -p $@

###################
# Rules to clean up

clean:
	@ -rm -rf $(OBJ_DIR)
	@ -rm -rf $(LIB_DIR)
	@ -rm -rf $(BIN_DIR)
	@ -rm -f $(PLEXILSIMULATOR_TARGET)
