# A very basic top-level Makefile

default: all

all:
	@ cd universal-exec; jam
	@ cd luv; ant jar
	@ cd standard-plexil; jam
	@ cd interfaces; jam
	@ $(MAKE) -C apps
	@ cd checker; ant jar
	@ echo Done.

clean:
	@ cd universal-exec; jam $@
	@ cd luv; ant $@
	@ cd standard-plexil; jam $@
	@ cd interfaces; jam $@
	@ $(MAKE) -C apps $@
	@ cd checker; ant $@
	@ echo Done.

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: 
	$(MAKE) all
	@ cd CORBA; jam

corba-clean: 
	$(MAKE) clean
	@ cd CORBA; jam clean

