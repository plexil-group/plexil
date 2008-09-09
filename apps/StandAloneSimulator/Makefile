SRC_DIR   = .

LIB_DIR   = lib
OBJ_DIR   = objs

LIB_NAME = standalonesimulator

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
INCLUDES = -I$(ACE_ROOT)
ifdef SHARED
 LIB_EXT = .so
 ROBOSIMLIB +=  -L$(LIB_DIR) -l$(LIB_NAME)
 SHARED_OBJ_FLAGS += -fpic
 SHARED_LIB_FLAGS += -z combreloc -shared -Wl
else
 LIB_EXT = .a
endif

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)

LIB_OBJECTS = $(OBJ_DIR)/SimulatorScriptReader$(OBJ_EXT) \
	      $(OBJ_DIR)/Simulator$(OBJ_EXT) \
	      $(OBJ_DIR)/ResponseMessageManager$(OBJ_EXT) \
	      $(OBJ_DIR)/TimingService$(OBJ_EXT) \
	      $(OBJ_DIR)/ResponseFactory$(OBJ_EXT)


############################
# Rules to build the targets

all : $(LIB_DIR)/$(LIB_TARGET)
	(cd RoboSimSimulator; make)


ifdef SHARED
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS)
else
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)
endif

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_OBJ_FLAGS) -c $< -o $@

#########################################
# Rules that make the directory structure

$(OBJ_DIR) :
	-mkdir -p $@

$(LIB_DIR) :
	-mkdir -p $@

###################
# Rules to clean up

clean:
	@ -rm -rf $(OBJ_DIR)
	@ -rm -rf $(LIB_DIR)
	(cd RoboSimSimulator; make clean)
