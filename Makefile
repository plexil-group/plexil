# A very basic top-level Makefile

ifeq ($(PLEXIL_HOME),)
export PLEXIL_HOME := $(shell pwd)
endif

default: all

tinyxml:
	$(MAKE) -C third-party/tinyxml -f Makefile.plexil

sockets:
	$(MAKE) -C src/interfaces/Sockets

utils: tinyxml
	$(MAKE) -C src/utils

exec-core: utils
	$(MAKE) -C src/exec

LuvListener: exec-core sockets
	$(MAKE) -C src/interfaces/LuvListener

luv:
	(cd luv && ant jar)

app-framework: exec-core sockets
	$(MAKE) -C src/app-framework

TestExec: exec-core LuvListener luv
	$(MAKE) -C src/apps/TestExec

TestExecSAS: exec-core app-framework sockets luv
	$(MAKE) -c src/apps/TestExecSAS
	$(MAKE) -C src/apps/StandAloneSimulator plexilsim

all: TestExec TestExecSAS standard-plexil checker

clean:
	$(MAKE) -C third-party/tinyxml $@
	$(MAKE) -C src/utils $@
	$(MAKE) -C src/exec $@
	$(MAKE) -C src/interfaces/lcm-structs $@
	$(MAKE) -C src/interfaces/LuvListener $@
	$(MAKE) -C src/interfaces/Sockets $@
	$(MAKE) -C src/app-framework $@
	$(MAKE) -C src/apps/StandAloneSimulator $@
	$(MAKE) -C src/apps/TestExec $@
	$(MAKE) -C src/apps/TestExecSAS $@
	(cd standard-plexil && jam $@)
	(cd luv && ant $@)
# KMD: there is no 'clean' target here
#	- $(MAKE) -C interfaces/lcm $@
	(cd checker && ant $@)
	@ echo Done.

# Convenience targets

standard-plexil:
	(cd standard-plexil && jam)

#TestMultiExec: luv
#	(cd universal-exec; jam)
#	$(MAKE) -C interfaces all
#	(cd app-framework; jam)
#	(cd apps/TestMultiExec; jam)

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: 
	$(MAKE) all
	@ cd CORBA; jam

corba-clean: 
	$(MAKE) clean
	@ cd CORBA; jam clean

