# Translates Extended PLEXIL test files into Core PLEXIL, placing the
# output in the 'translated' subdirectory.

SHELL = /bin/sh

SYNCH =	SynchronousCommand1 SynchronousCommand2 SynchronousCommand3 \
	SynchronousCommand4 SynchronousCommand5 SynchronousCommand6 \
	SynchronousCommand7 SynchronousCommand8 SynchronousCommand9 \
	SynchronousCommand10

REST =  UncheckedSequence1 UncheckedSequence2 \
	UncheckedSequence3 UncheckedSequence4 \
	Sequence1 Sequence2 Sequence3 Sequence4 \
	Concurrence1 Concurrence2 Concurrence3 Concurrence4 Concurrence5 \
    If1 If2 If3 If4 If5 \
	While1 While2 While3 While4 \
    For1 For2 For3 For4 For5 \
	Try1 Try2 Try3 Try4 \
	NodeCheck1 MessageReceived1 Sequence-Concurrence OnMessage OnCommand \
	OnCommandSequence OnCommandStartCondition OnCommandSkipCondition \
	Lookup1 Lookup2 Wait1 Wait2 Wait3 Interface

TESTS = $(SYNCH) $(REST)


# Temporary file
DIFF = /tmp/plexil-$$$$

default: test

test:
	@ echo Translating Extended Plexil tests...
	@ for i in $(TESTS); do \
	        eplexil $$i.epx translated/$$i.plx ; \
		xmllint --noout --schema ../core-plexil.xsd translated/$$i.plx ; \
		diff translated/$$i.plx translated/verified-$$i.plx \
		  > $(DIFF) ; \
		diffsize=`wc $(DIFF) | awk '{print $$1}'` ; \
		if [ $$diffsize -eq 0 ] ; then \
		  echo "     Verified $$i.plx" ; \
		else \
		  echo WARNING: $$i.plx does not verify! ; \
		fi ; \
	  done
	@ echo Finished.

# For developers: Check the current output against the benchmarks.
checkall:
	@ for i in $(TESTS); do \
		./check $$i ; \
	  done

# For developers: Bless the current output, making it the new benchmark.
passall:
	@ for i in $(TESTS); do \
		./pass $$i ; \
	  done

# For developers: Clean out the generated files.
clean:
	@for test in $(TESTS); do \
		$(RM) translated/$$test.plx ; \
	done
