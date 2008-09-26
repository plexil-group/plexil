ifndef SSWGCOMM_HOME
	SSWGCOMM_HOME = $(PLEXIL_HOME)/apps/robosim/RobotSimulator/SSWGComm
endif

ifndef STANDALONE_SIMULATOR_HOME
	STANDALONE_SIMULATOR_HOME = $(PLEXIL_HOME)/apps/StandAloneSimulator
endif

SRC_DIR   = .

LIBS = -lpthread  -L$(ACE_ROOT)/lib -lACE

ifdef SHARED
 LIBS += -L$(STANDALONE_SIMULATOR_HOME)/lib -lstandalonesimulator \
	 -L$(SSWGCOMM_HOME)/Unix/lib -lsswgcomm
else
  LIBS += $(STANDALONE_SIMULATOR_HOME)/lib/libstandalonesimulator.a \
	 $(SSWGCOMM_HOME)/Unix/lib/libsswgcomm.a 
endif

BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
INCLUDES = -I$(STANDALONE_SIMULATOR_HOME) -I$(ACE_ROOT)  -I$(SSWGCOMM_HOME)

ifdef SHARED
 LIB_EXT = .so
 SHARED_OBJ_FLAGS += -fpic
 SHARED_LIB_FLAGS += -z combreloc -shared -Wl
else
 LIB_EXT = .a
endif

ROBOSIMSIMULATOR_LIBS += $(LIB_DIR)/$(LIB_TARGET)

LIB_NAME = robosimsimulator

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIMSIMULATOR_TARGET = roboSimSimulator

LIB_OBJECTS = $(OBJ_DIR)/RoboSimResponseFactory$(OBJ_EXT) \
	      $(OBJ_DIR)/SSWGCommRelay$(OBJ_EXT)

ROBOSIMSIMULATOR_OBJECTS = $(OBJ_DIR)/RoboSimSimulator$(OBJ_EXT)


############################
# Rules to build the targets

all : $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(ROBOSIMSIMULATOR_TARGET)

ifdef SHARED
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS)
else
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)
endif

$(BIN_DIR)/$(ROBOSIMSIMULATOR_TARGET) : $(BIN_DIR) $(ROBOSIMSIMULATOR_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(ROBOSIMSIMULATOR_OBJECTS) $(ROBOSIMSIMULATOR_LIBS) $(LIBS)


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
	@ -rm -f $(ROBOSIMSIMULATOR_TARGET)
