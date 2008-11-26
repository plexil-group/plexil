# A very basic top-level Makefile

default: all

all:
	@ cd universal-exec; jam
	@ cd luv; ant jar
	@ cd standard-plexil; jam
	@ cd interfaces; jam
	@ cd apps; make
	@ cd checker; ant
	@ echo Done.

clean:
	@ cd universal-exec; jam clean
	@ cd luv; ant clean
	@ cd standard-plexil; jam clean
	@ cd interfaces; jam clean
	@ cd apps; make clean
	@ cd checker; ant clean
	@ echo Done.

# The following targets apply only when the UE is being used with an
# ACE/TAO Corba installation.

corba: 
	$(MAKE) all
	@ cd CORBA; jam

corba-clean: 
	$(MAKE) clean
	@ cd CORBA; jam clean

