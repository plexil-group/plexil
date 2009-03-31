# Translates Extended PLEXIL files into Core PLEXIL.

SHELL = /bin/sh

TESTS = UncheckedSequence1 UncheckedSequence2 UncheckedSequence3 UncheckedSequence4 \
	Sequence1 Sequence2 Sequence3 Sequence4 \
	Try1 Try2 Try3 Try4 If1 If2 If3  While1 While2 While3 For1 For2 For3

# Temporary file
DIFF = /tmp/plexil-$$$$

default: test

test:
	@ echo Translating Extended Plexil tests...
	@ for i in $(TESTS); do \
	        eplexil $$i.epx translated/$$i.plx ; \
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

# For developers: Check the current output agains the benchmarks.
checkall:
	@ for i in $(TESTS); do \
		./check $$i ; \
	  done

# For developers: Bless the current output, making it the new benchmark.
passall:
	@ for i in $(TESTS); do \
		./pass $$i ; \
	  done
