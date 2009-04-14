# A simple Makefile for building the Plexil apps.

# NOTE: robosim is excluded because it requires Open GL.  Build it
# separately if you want it.

default:
	@ ( cd TestExec; jam )
	@ $(MAKE) -C StandAloneSimulator simcore SHARED=1
	@ $(MAKE) -C StandAloneSimulator/PlexilSimulator

clean:
	@ ( cd TestExec; jam $@ )
	@ ( cd StandAloneSimulator; make $@ )
