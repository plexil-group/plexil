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

demo: ../libUdpAdapter.$(DYLIB)
	@echo ' '
	$(RUN_AGENTS) $(OPTS) test-recv -c udp test-send -c udp

timeout1: ../libUdpAdapter.$(DYLIB) test-recv.plx
	@echo ' '
	plexilexec -p test-recv.plx -c udp.xml

timeout2: ../libUdpAdapter.$(DYLIB) test-send.plx
	@echo ' '
	plexilexec -p test-send.plx -c udp.xml

../libUdpAdapter.$(DYLIB):
	$(MAKE) -C ../

cleaner: clean
	make -C ../ cleaner

.PHONY: demo

# EOF
