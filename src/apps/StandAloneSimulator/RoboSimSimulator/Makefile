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

ifndef SSWGCOMM_HOME
	SSWGCOMM_HOME = $(PLEXIL_HOME)/apps/robosim/RobotSimulator/SSWGComm
endif

ifndef STANDALONE_SIMULATOR_HOME
	STANDALONE_SIMULATOR_HOME = ..
endif

SRC_DIR   = .

LIBS = -lpthread

OSTYPE   ?= $(shell uname -s)
MACHTYPE ?= $(shell uname -p)
OS_ARCH  = $(OSTYPE)-$(MACHTYPE)

ifdef SHARED
 LIBS += -L$(STANDALONE_SIMULATOR_HOME)/lib -lstandalonesimulator \
	 -L$(SSWGCOMM_HOME)/$(OS_ARCH)/lib -lsswgcomm
else
  LIBS += $(STANDALONE_SIMULATOR_HOME)/lib/libstandalonesimulator.a \
	 $(SSWGCOMM_HOME)/$(OS_ARCH)/lib/libsswgcomm.a 
endif

BIN_DIR   = bin
LIB_DIR   = lib
OBJ_DIR   = objs

CXX     = g++
OBJ_EXT = .o
CXXFLAGS += -g -Wall -fno-rtti -Wno-deprecated
INCLUDES = -I$(STANDALONE_SIMULATOR_HOME) -I$(SSWGCOMM_HOME)

ifdef SHARED
 LIB_EXT = .so
 SHARED_OBJ_FLAGS += -fpic
 ifeq ($(findstring Darwin, $(OSTYPE)), Darwin)
  SHARED_LIB_FLAGS += -dynamiclib -Wl
 else
  SHARED_LIB_FLAGS += -z combreloc -shared -Wl
 endif
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
	$(CXX) $(CXXFLAGS) $(SHARED_OBJ_FLAGS) $(SHARED_LIB_FLAGS) -o $@ $(LIB_OBJECTS) $(LIBS)
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
