# Translates Extended PLEXIL test files into Core PLEXIL, placing the
# output in the 'translated' subdirectory.

SHELL = /bin/sh

SYNCH =	SynchronousCommand1 SynchronousCommand2 SynchronousCommand3 \
	SynchronousCommand4 SynchronousCommand5 SynchronousCommand6 \
	SynchronousCommand7 SynchronousCommand8 SynchronousCommand9 \
	SynchronousCommand10

REST = UncheckedSequence1 UncheckedSequence2 \
	UncheckedSequence3 UncheckedSequence4 \
	Sequence1 Sequence2 Sequence3 Sequence4 \
	Concurrence1 Concurrence2 Concurrence3 Concurrence4 Concurrence5 \
    If1 If2 If3 If4 If5 If6 \
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

output:
	@ mkdir -p $@

test: output
	@ test_failed=0; \
	for i in $(TESTS); do \
		eplexil input/$$i.epx output/$$i.plx ; \
		if [ $$? -ne 0 ] ; then \
			test_failed=1 ; \
			echo "*** Translation of input/$$i.epx failed ***" ;\
			exit 1 ; \
		fi ; \
		xmllint --noout --schema ../core-plexil.xsd output/$$i.plx ; \
		if [ $$? -ne 0 ] ; then test_failed=1 ; fi ; \
		diff output/$$i.plx verified/verified-$$i.plx \
		  > $(DIFF) ; \
		diffsize=`wc $(DIFF) | awk '{print $$1}'` ; \
		if [ $$diffsize -ne 0 ] ; then \
			test_failed=1 ; \
			echo "*** output/$$i.plx failed verification ***" ; \
		fi ; \
	done ; \
	if [ $$test_failed -ne 0 ] ; then echo 'Test(s) failed.' ; exit 1 ; fi 
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
	@ $(RM) -rf output

.PHONY: test clean
