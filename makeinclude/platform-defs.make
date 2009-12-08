# Conditionalizing make variables by platform

OSNAME		:= $(shell uname -s)
OSVERSION	:= $(shell uname -r)
ARCH		:= $(shell uname -p)

#$(info OSNAME is $(OSNAME), OSVERSION is $(OSVERSION), ARCH is $(ARCH))

include $(PLEXIL_HOME)/makeinclude/platform-$(OSNAME).make
