# A simple Makefile for building the Plexil apps.

# NOTE: robosim is excluded because it requires Open GL.  Build it
# separately if you want it.

default:
	@ cd TestExec; jam
	@ cd StandAloneSimulator; $(MAKE) simcore SHARED=1

clean:
	@ cd TestExec; jam clean
	@ cd StandAloneSimulator; $(MAKE) clean
