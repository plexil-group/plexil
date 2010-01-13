# Copyright (c) 2006-2009, Universities Space Research Association (USRA).
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Universities Space Research Association nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SRC_DIR   = .

OSTYPE   ?= $(shell uname -s)
MACHTYPE ?= $(shell uname -p)
OS_ARCH = $(OSTYPE)-$(MACHTYPE)

ifeq ($(findstring Darwin, $(OSTYPE)), Darwin)
	DEFINES = -DDarwin
	OPENGL_LIBS += -framework OpenGL -framework GLUT
else
	ifeq ($(MACHTYPE),x86_64)
		LIBS = -L/usr/lib64 
	endif
#	X_LIBS = -Xi -Xmu
	X_LIBS = -L/usr/X11R6/lib -L/usr/lib -lXi -lXmu
	OPENGL_LIBS += -lGL -lGLU -lglut
endif


BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs
LIB_NAME = robosim

LIBS = -lpthread 

ifdef SHARED
 LIBS += -L../SSWGComm/$(OS_ARCH)/lib -lsswgcomm
else
 LIBS += ../SSWGComm/$(OS_ARCH)/lib/libsswgcomm.a
endif

CXX	= g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
ifdef SHARED
 LIB_EXT = .so
 ROBOSIMLIB +=  -L$(LIB_DIR) -l$(LIB_NAME)
 SHARED_OBJ_FLAGS += -fpic
 ifeq ($(findstring Darwin, $(OSTYPE)), Darwin)
  SHARED_LIB_FLAGS += -dynamiclib -Wl
 else
  SHARED_LIB_FLAGS += -z combreloc -shared -Wl
 endif
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
	$(CXX) $(CXXFLAGS) $(DEFINES) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS) $(LIBS) $(OPENGL_LIBS)
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
