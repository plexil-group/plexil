# A very basic top-level Makefile

default: all

all: luv standard-plexil
	@ cd universal-exec; jam
	@ cd standard-plexil; jam
	@ cd luv; ant jar
	@ cd interfaces; jam
	@ $(MAKE) -C apps
	@ cd app-framework; jam
	@ cd checker; ant jar
	@ echo Done.

clean:
	@ cd universal-exec; jam $@
	@ cd standard-plexil; jam $@
	@ cd luv; ant $@
	@ cd interfaces; jam $@
	@ $(MAKE) -C apps $@
	@ cd app-framework; jam $@
	@ cd checker; ant $@
	@ echo Done.

# Convenience targets

# Coming soon
#links:
#	$(MAKE) -C bin $@
#	$(MAKE) -C lib $@

luv:
	(cd luv; ant jar)

standard-plexil:
	(cd standard-plexil; jam)

TestExec: luv
	(cd universal-exec; jam)
	(cd interfaces; jam)
	(cd apps/TestExec; jam)

TestExecSAS: luv
	(cd universal-exec; jam)
	(cd interfaces; jam)
	(cd app-framework; jam)
	(cd apps/TestExecSAS; jam)
	$(MAKE) -C apps/StandAloneSimulator plexilsim

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: 
	$(MAKE) all
	@ cd CORBA; jam

corba-clean: 
	$(MAKE) clean
	@ cd CORBA; jam clean

