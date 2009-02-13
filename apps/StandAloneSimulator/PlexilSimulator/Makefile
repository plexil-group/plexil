ifndef STANDALONE_SIMULATOR_HOME
	STANDALONE_SIMULATOR_HOME = $(PLEXIL_HOME)/apps/StandAloneSimulator
endif

SRC_DIR   = .

LIBS = -lpthread

ifdef SHARED
 LIBS += -L$(STANDALONE_SIMULATOR_HOME)/lib -lstandalonesimulator  -L../../../interfaces/lib/
else
  LIBS += $(STANDALONE_SIMULATOR_HOME)/lib/libstandalonesimulator.a -L../../../interfaces/lib/
endif

BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
INCLUDES = -I$(STANDALONE_SIMULATOR_HOME) -I../../../interfaces/Sockets/base

ifdef SHARED
 LIB_EXT = .so
 SHARED_OBJ_FLAGS += -fpic
 SHARED_LIB_FLAGS += -z combreloc -shared -Wl
else
 LIB_EXT = .a
endif

PLEXILSIMULATOR_LIBS += $(LIB_DIR)/$(LIB_TARGET)

LIB_NAME = plexilsimulator

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
PLEXILSIMULATOR_TARGET = plexilSimulator

LIB_OBJECTS = $(OBJ_DIR)/PlexilSimResponseFactory$(OBJ_EXT) \
	      $(OBJ_DIR)/PlexilCommRelay$(OBJ_EXT) \
	      $(OBJ_DIR)/ThreadSpawn$(OBJ_EXT)

PLEXILSIMULATOR_OBJECTS = $(OBJ_DIR)/PlexilSimulator$(OBJ_EXT)


############################
# Rules to build the targets

all : $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(PLEXILSIMULATOR_TARGET)

ifdef SHARED
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS)
else
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)
endif

$(BIN_DIR)/$(PLEXILSIMULATOR_TARGET) : $(BIN_DIR) $(PLEXILSIMULATOR_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(PLEXILSIMULATOR_OBJECTS) $(PLEXILSIMULATOR_LIBS) $(LIBS) -lSockets_g


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
