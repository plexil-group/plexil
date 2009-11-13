ifndef ROBOSIM_HOME
	ROBOSIM_HOME = ../RobotSimulator
endif

SRC_DIR   = .

INCLUDES += -I$(PLEXIL_HOME)/universal-exec/TinyXml/base -I$(PLEXIL_HOME)/universal-exec/Utils -I$(PLEXIL_HOME)/universal-exec/Utils/base -I$(PLEXIL_HOME)/universal-exec/Exec -I$(PLEXIL_HOME)/universal-exec/Exec/base -I$(PLEXIL_HOME)/universal-exec/Exec/component -I$(PLEXIL_HOME)/interfaces/Sockets/base  -I$(PLEXIL_HOME)/interfaces/Common/component -I$(PLEXIL_HOME)/interfaces/LuvListener/base -I$(PLEXIL_HOME)/interfaces/Common/base -I$(ROBOSIM_HOME)/SSWGComm

LIB_DIRS += -L$(PLEXIL_HOME)/universal-exec/lib  -L$(PLEXIL_HOME)/interfaces/lib 

LIBS += -lpthread -lLuvListener_g -lPlexilInterface_g -lPlexilUtils_g -lPlexilTinyXml_g -lSockets_g -lExec_g

OSTYPE   ?= $(shell uname -s)
MACHTYPE ?= $(shell uname -p)
OS_ARCH = $(OSTYPE)-$(MACHTYPE)

ifdef SHARED
 LIBS += -L$(ROBOSIM_HOME)/SSWGComm/$(OS_ARCH)/lib/ -lsswgcomm
else
 LIBS += $(ROBOSIM_HOME)/SSWGComm/$(OS_ARCH)/lib/libsswgcomm.a
endif

BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs

LIB_NAME = roboSimExec

CXX	= g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -Wno-deprecated
LIB_EXT = .a	
ROBOSIMEXEC_LIBS += $(LIB_DIR)/$(LIB_TARGET)

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIMEXEC_TARGET = roboSimExec

LIB_OBJECTS   = $(OBJ_DIR)/RoboSimInterfaceAdaptor$(OBJ_EXT)

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIMEXEC_OBJECTS = $(OBJ_DIR)/RoboSimExec.o

all : $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(ROBOSIMEXEC_TARGET)

$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)

$(BIN_DIR)/$(ROBOSIMEXEC_TARGET) : $(BIN_DIR) $(ROBOSIMEXEC_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(ROBOSIMEXEC_OBJECTS) $(ROBOSIMEXEC_LIBS) $(LIB_DIRS) $(LIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


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
	@ -rm -f roboSimExec
