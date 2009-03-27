# A simple Makefile for building the Plexil apps.

# NOTE: robosim is excluded because it requires Open GL.  Build it
# separately if you want it.
# NOTE: StandAloneSimulator depends on SSWGComm in robosim!

default:
	@ ( cd TestExec; jam )
#	@ $(MAKE) -C StandAloneSimulator simcore SHARED=1

clean:
	@ ( cd TestExec; jam $@ )
	@ $(MAKE) -C StandAloneSimulator $@
