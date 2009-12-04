
ifneq ($@,clean)
ifeq ($(ACE_ROOT),)
$(error The environment variable ACE_ROOT is not set. Exiting.)
endif

ifeq ($(TAO_ROOT),)
TAO_ROOT	:= $(ACE_ROOT)/TAO
endif
endif

all: corba-utils EventChannelReporter

corba-utils:
	$(MAKE) -C utils all

EventChannelReporter: utils
	$(MAKE) -C EventChannelReporter all

clean:
	$(MAKE) -C utils $@
	$(MAKE) -C EventChannelReporter $@

