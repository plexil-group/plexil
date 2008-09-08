SRC_DIR   = .

ifeq ($(OSTYPE),darwin)
	DEFINES = -DDarwin
	OPENGL_LIBS = -framework OpenGL -framework GLUT
else
	ifeq ($(MACHTYPE),x86_64)
		LIBS = -L/usr/lib64 
	endif
#	X_LIBS = -Xi -Xmu
	X_LIBS = -L/usr/X11R6/lib -lXi -lXmu
	OPENGL_LIBS += -lGL -lGLU -lglut
endif


BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs
LIB_NAME = robosim

LIBS = -lpthread 

ifdef SHARED
 LIBS += -L../SSWGComm/Unix/lib -lsswgcomm
else
 LIBS += ../SSWGComm/Unix/lib/libsswgcomm.a
endif

CXX	= g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
ifdef SHARED
 LIB_EXT = .so
 ROBOSIMLIB +=  -L$(LIB_DIR) -l$(LIB_NAME)
 SHARED_OBJ_FLAGS += -fpic
 SHARED_LIB_FLAGS += -z combreloc -shared -Wl
else
 LIB_EXT = .a
endif

ROBOSIM_LIBS += $(LIB_DIR)/$(LIB_TARGET)
LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIM_TARGET = robotSimulator
TESTEXEC_TARGET = testExec

LIB_OBJECTS   = $(OBJ_DIR)/MazeTerrain$(OBJ_EXT) \
		$(OBJ_DIR)/Goals$(OBJ_EXT) \
		$(OBJ_DIR)/EnergySources$(OBJ_EXT) \
		$(OBJ_DIR)/Robot$(OBJ_EXT) \
		$(OBJ_DIR)/RobotPositionServer$(OBJ_EXT)

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
ROBOSIM_OBJECTS = $(OBJ_DIR)/RobotSimulator.o
TESTEXEC_OBJECTS = $(OBJ_DIR)/TestExec.o


############################
# Rules to build the targets

all : $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(ROBOSIM_TARGET) $(BIN_DIR)/$(TESTEXEC_TARGET)


ifdef SHARED
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS)
else
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)
endif

$(BIN_DIR)/$(ROBOSIM_TARGET) : $(BIN_DIR) $(ROBOSIM_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(ROBOSIM_OBJECTS) $(OPENGL_LIBS) $(ROBOSIM_LIBS) $(LIBS) $(X_LIBS)
$(BIN_DIR)/$(TESTEXEC_TARGET) : $(BIN_DIR) $(TESTEXEC_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(TESTEXEC_OBJECTS) $(LIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_OBJ_FLAGS) -c $< -o $@


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
	@ -rm -f testExec robotSimulator
