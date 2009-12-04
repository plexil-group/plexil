# A very basic top-level Makefile

ifeq ($(PLEXIL_HOME),)
export PLEXIL_HOME := $(shell pwd)
endif

default: all

all: TestExec TestExecSAS standard-plexil checker

TestExec: exec-core LuvListener luv
	$(MAKE) -C src/apps/TestExec

TestExecSAS: lcm-structs exec-core app-framework sockets luv
	$(MAKE) -C src/apps/TestExecSAS
	$(MAKE) -C src/apps/StandAloneSimulator plexilsim

checker:
	(cd src/checker && ant jar)

luv:
	(cd src/luv && ant jar)

standard-plexil:
	(cd src/standard-plexil && ant install)

tinyxml:
	$(MAKE) -C third-party/tinyxml -f Makefile.plexil

utils: tinyxml
	$(MAKE) -C src/utils

exec-core: utils
	$(MAKE) -C src/exec

LuvListener: exec-core sockets
	$(MAKE) -C src/interfaces/LuvListener

app-framework: exec-core sockets
	$(MAKE) -C src/app-framework

sockets:
	$(MAKE) -C src/interfaces/Sockets

lcm:
	(cd third-party/lcm && \
 ./configure --prefix=$(PLEXIL_HOME) --without-python --without-java && \
 $(MAKE))

lcm-structs: lcm
	$(MAKE) -C src/interfaces/lcm-structs

clean:
	$(MAKE) -C third-party/tinyxml $@
	# should only be an error if 'configure' hasn't been run yet
	-$(MAKE) -C third-party/lcm $@
	$(MAKE) -C src/utils $@
	$(MAKE) -C src/exec $@
	$(MAKE) -C src/interfaces/lcm-structs $@
	$(MAKE) -C src/interfaces/LuvListener $@
	$(MAKE) -C src/interfaces/Sockets $@
	$(MAKE) -C src/CORBA $@
	$(MAKE) -C src/app-framework $@
	$(MAKE) -C src/apps/StandAloneSimulator $@
	$(MAKE) -C src/apps/TestExec $@
	$(MAKE) -C src/apps/TestExecSAS $@
	(cd src/standard-plexil && ant $@)
	(cd src/luv && ant $@)
	(cd src/checker && ant $@)
	@ echo Done.

# Convenience targets

#TestMultiExec: luv
#	(cd universal-exec; jam)
#	$(MAKE) -C interfaces all
#	(cd app-framework; jam)
#	(cd apps/TestMultiExec; jam)

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: utils exec app-framework
	$(MAKE) -C src/CORBA all

corba-utils: utils
	$(MAKE) -C src/CORBA $@

