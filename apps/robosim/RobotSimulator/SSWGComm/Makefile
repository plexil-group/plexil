SRC_DIR   = .

ifdef VXWORKS
  OSTYPE = VxWorks
  OS_ARCH = $(OSTYPE)
else
  OSTYPE   = $(shell uname -s)
  MACHTYPE = $(shell uname -p)
  OS_ARCH = $(OSTYPE)-$(MACHTYPE)
endif

BIN_DIR   = $(OS_ARCH)/bin
LIB_DIR   = $(OS_ARCH)/lib
OBJ_DIR   = $(OS_ARCH)/objs

LIB_NAME = sswgcomm

ifdef VXWORKS
  CXX 	= $(WIND_GNU_PATH)/x86-linux2/bin/c++ppc
  BIN_EXT = .vxe
  CXXFLAGS += -O2 -mhard-float -mstrict-align -mregnames -ansi -mrtp  -fstrength-reduce -fno-builtin
  CXXFLAGS += -Werror -Wno-ctor-dtor-privacy -Wno-unknown-pragmas -Wunused
  INCLUDES += -I$(WIND_HOME)/vxworks-6.3/target/usr/h/ \
	      -I$(WIND_HOME)/vxworks-6.3/target/usr/h/wrn/coreip
  DEFINES  += -DCPU=PPC32 -DTOOL_FAMILY=gnu -DTOOL=gnu
  LIBS += -lstdc++ -L$(WIND_HOME)/vxworks-6.3/target/usr/lib/ppc/PPC32/common
 ifdef SHARED
   OBJ_EXT = .sho
   LIB_EXT = .so
   SHARED_APP_FLAGS += -non-static -Wl,-rpath,/romfs/lib
   SHARED_OBJ_FLAGS += -fpic
   SHARED_LIB_FLAGS += -shared -Wl,-soname,$(LIB_TARGET) \
	               -L$(WIND_HOME)/vxworks-6.3/target/usr/lib/ppc/PPC32/common/PIC
   SSWGLIB += -Wl,--start-group $(LIB_DIR)/$(LIB_TARGET) -L$(LIB_DIR) -l$(LIB_NAME) -Wl,--end-group
 else
   OBJ_EXT = .o
   LIB_EXT = .a
   SSWGLIB += $(LIB_DIR)/$(LIB_TARGET)
 endif
else
  # not VxWorks
  CXX	= g++
  OBJ_EXT = .o
  CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
  DEFINES  += -DUNIX
 ifdef SHARED
  LIB_EXT = .so
  SSWGLIB +=  -L$(LIB_DIR) -l$(LIB_NAME)
  SHARED_OBJ_FLAGS += -fpic
  ifeq ($(findstring Darwin, $(OS)), Darwin)
   SHARED_LIB_FLAGS += -dynamiclib -Wl
  else
   SHARED_LIB_FLAGS += -z combreloc -shared -Wl
  endif
 else
  LIB_EXT = .a
  SSWGLIB += $(LIB_DIR)/$(LIB_TARGET)
 endif
 LIBS += -lpthread
endif

APP_TARGET = testComm$(BIN_EXT)
APP1_TARGET = testServer$(BIN_EXT)
APP2_TARGET = testClient1$(BIN_EXT)
APP3_TARGET = testClient2$(BIN_EXT)

LIB_OBJECTS   = $(OBJ_DIR)/SSWGClient$(OBJ_EXT) \
		$(OBJ_DIR)/SSWGServer$(OBJ_EXT) \
		$(OBJ_DIR)/ThreadSpawn$(OBJ_EXT)

LIB_TARGET = lib$(LIB_NAME)$(LIB_EXT)
APP_OBJECTS = $(OBJ_DIR)/TestComm.o
APP1_OBJECTS = $(OBJ_DIR)/TestServer.o
APP2_OBJECTS = $(OBJ_DIR)/TestClient1.o
APP3_OBJECTS = $(OBJ_DIR)/TestClient2.o

############################
# Rules to build the targets

all : env_check $(LIB_DIR)/$(LIB_TARGET) $(BIN_DIR)/$(APP_TARGET) $(BIN_DIR)/$(APP1_TARGET) $(BIN_DIR)/$(APP2_TARGET) $(BIN_DIR)/$(APP3_TARGET)

env_check:
 ifdef VXWORKS
	@echo ' Building in VxWorks.'
 ifndef WIND_HOME
	@echo ' The environment variable WIND_HOME has not been set. Run wrenv.sh first.'
	@exit 1;
 endif
 else
	@echo ' Building in Unix.'
 endif

ifdef SHARED
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS)
else
$(LIB_DIR)/$(LIB_TARGET) : $(OBJ_DIR) $(LIB_DIR) $(LIB_OBJECTS)
	$(AR) ruc $@ $(LIB_OBJECTS)
endif

$(BIN_DIR)/$(APP_TARGET) : $(BIN_DIR) $(APP_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(APP_OBJECTS) $(LIBS) $(SSWGLIB)

$(BIN_DIR)/$(APP1_TARGET) : $(BIN_DIR) $(APP1_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(APP1_OBJECTS) $(LIBS) $(SSWGLIB)

$(BIN_DIR)/$(APP2_TARGET) : $(BIN_DIR) $(APP2_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(APP2_OBJECTS) $(LIBS) $(SSWGLIB)

$(BIN_DIR)/$(APP3_TARGET) : $(BIN_DIR) $(APP3_OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_APP_FLAGS) -o $@ $(APP3_OBJECTS) $(LIBS) $(SSWGLIB)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(OBJ_DIR)/%.sho : $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

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

clean :
	@ -rm -rf $(OBJ_DIR)
	@ -rm -rf $(LIB_DIR)
	@ -rm -rf $(BIN_DIR)
	@ -rm -rf $(OS_ARCH)
	@ -rm -rf VxWorks


#############################
# Hand generated dependencies
