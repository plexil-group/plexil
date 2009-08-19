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

lcm:
	(cd lcm-0.2.1; ./configure --without-java --without-python --prefix=`echo $$PWD`)
	$(MAKE) -C lcm-0.2.1
