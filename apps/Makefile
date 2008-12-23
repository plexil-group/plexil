# A simple Makefile for building the Plexil apps.

# NOTE: robosim is excluded because it requires Open GL.  Build it
# separately if you want it.

default:
	@ cd TestExec; jam
	@ cd StandAloneSimulator; $(MAKE)

clean:
	@ cd TestExec; jam clean
	@ cd robosim; $(MAKE) clean
	@ cd StandAloneSimulator; $(MAKE) clean
