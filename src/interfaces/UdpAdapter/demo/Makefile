# -*- Mode: Makefile -*-
# File: plexil/src/interfaces/UdpAdapter/demo/Makefile
# Note: Makefile for the UdpAdapter demo

ifeq ($(shell uname),Darwin)
  DYLIB=dylib
else
  DYLIB=so
endif

OPTS= -no-ipc #-v -x -vx -d

include $(PLEXIL_HOME)/makeinclude/generic-plexil.make

demo: ../UdpAdapter.$(DYLIB)
	@echo ' '
	$(RUN_AGENTS) $(OPTS) test-recv -c udp test-send -c udp

timeout: ../UdpAdapter.$(DYLIB) test-recv.plx
	@echo ' '
	plexilexec -p test-recv.plx -c udp.xml

../UdpAdapter.$(DYLIB):
	$(MAKE) -C ../

.PHONY: demo

# EOF
