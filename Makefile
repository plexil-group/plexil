# A very basic top-level Makefile

default: all

corba: 
	$(MAKE) all
	@ cd CORBA; jam

all:
	@ cd universal-exec; jam
	@ cd luv; ant jar
	@ cd standard-plexil; jam
	@ cd interfaces; jam
	@ cd apps; make
	@ echo Done.

clean:
	@ cd universal-exec; jam clean
	@ cd luv; ant clean
	@ cd standard-plexil; jam clean
	@ cd interfaces; jam clean
	@ cd apps; make clean
	@ echo Done.
