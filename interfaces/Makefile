SHELL = /bin/sh

default: non-lcm

all:
	@ $(MAKE) non-lcm
	@ $(MAKE) lcm

clean:
	jam clean
	$(MAKE) -C lcm-0.2.1 $@

non-lcm:
	jam

lcm: lcm-0.2.1/Makefile
	$(MAKE) -C lcm-0.2.1
# we expect this one to blow up
	- $(MAKE) -C lcm-0.2.1 install

lcm-0.2.1/Makefile:
	(cd lcm-0.2.1; ./configure --without-java --without-python --prefix=`echo $$PWD`)
