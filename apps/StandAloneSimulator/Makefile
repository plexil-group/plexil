SRC_DIR   = .

LIB_DIR   = lib
OBJ_DIR   = objs

LIB_NAME = standalonesimulator

#OSTYPE ?= $(shell uname -s)
OSTYPE = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
MACHTYPE ?= $(shell uname -p)

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
ifdef SHARED
 LIB_EXT = .so
 ifeq ($(findstring DARWIN, $(OSTYPE)), DARWIN)
  SHARED_LIB_FLAGS += -dynamiclib -Wl
  SHARED_OBJ_FLAGS += -fPIC
 else
  SHARED_LIB_FLAGS += -z combreloc -shared -Wl
  SHARED_OBJ_FLAGS += -fpic
 endif
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
	$(MAKE) -C RoboSimSimulator

simcore : $(LIB_DIR)/$(LIB_TARGET)

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
	@ $(MAKE) -C RoboSimSimulator $@
