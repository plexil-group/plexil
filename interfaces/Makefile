SHELL = /bin/sh

default: non-lcm

all:
	@ $(MAKE) non-lcm
	@ $(MAKE) lcm

clean:
	jam clean
# can fail if configure not yet executed
	- $(MAKE) -C lcm $@

non-lcm:
	jam

lcm: lcm/Makefile
	$(MAKE) -C lcm
# we expect this one to blow up because of directory permissions
	- $(MAKE) -C lcm install

lcm/Makefile:
	(cd lcm; ./configure --without-java --without-python --prefix=`echo $$PWD`)
