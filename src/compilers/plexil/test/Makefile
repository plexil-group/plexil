# Copyright (c) 2006-2011, Universities Space Research Association (USRA).
#  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#    * Neither the name of the Universities Space Research Association nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# PLEXIL_HOME			?= ../../../..

SMALL_TESTS = empty0 empty1 empty2 empty3 empty4 empty5 \
	      command0 command1 command2 command3 command4 \
	      sequence0 sequence1 sequence2 sequence3 \
	      assignment0 assignment1 assignment2 \
	      concurrence0 concurrence1 concurrence1 concurrence2 concurrence3 \
	      Fibs Fib PrintFib FibTest LibraryCall1 LibraryCall2 \
	      array-copy CopyReals \
              on-command1 on-command2 isKnown1 for1 for2 \
	      on-message1 on-message2 concat1 nodeState1 nodeState2 \
	      nodeOutcome1 command-handle1 interface-test interface-test1 \
	      DriveToSchool SafeDrive CruiseControl sync-cmd-test \
	      simpleIf simpleIfThenElse simpleAssignment while1 AddArray \
              Decrement Increment Sum RoboSimSimple2 SimpleDrive \
              array-add array-complex array-copy  basic-wait-test boolean1 \
	      duplicate-node-name ifWithClauseIds DriveToTarget Exchange \
              library-call1 library-node1 literal-format-test if2 if3

BROKEN       =  array-library-call LibraryCallWithArray resource0 Resource1 Resource2 Resource3 Resource4
INVALID_BUT_PASS  = on-message3

TESTS	         := $(SMALL_TESTS)
#TESTS	         := Resource3 Resource4


# The VALID_TEST* macros are used by the original test framework.  

VALID_TESTS	:= $(TESTS)

VALID_TEST_FILES := $(VALID_TESTS:%=input/%.ple)

default: test

all: setup valid-tests syntax-error-tests semantic-error-tests

setup:
	-$(RM) tempRegressionResults

valid-tests: $(VALID_TEST_FILES) | output setup
	touch tempRegressionResults ; \
	for tf in $(VALID_TEST_FILES) ; do \
		echo $${tf} ; \
		./validate $${tf} 2>> tempRegressionResults ; \
		if [ $$? -ne 0 ] ; then failure=1 ; fi; \
	done ; \
	exit $${failure}

# Temporary file
DIFF = /tmp/plexil-$$$$

test:
	@ echo Translating Standard Plexil tests...
	@ for i in $(TESTS); do \
	        ../PlexilCompiler --epx-only -o output/$$i.epx input/$$i.ple ; \
		diff output/$$i.epx output/valid-$$i.epx \
		  > $(DIFF) ; \
		diffsize=`wc $(DIFF) | awk '{print $$1}'` ; \
		if [ $$diffsize -eq 0 ] ; then \
		  echo "     Verified $$i.epx" ; \
		else \
		  echo WARNING: $$i.epx does not verify! ; \
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

syntax-error-tests:

semantic-error-tests:

output:
	-mkdir -p output

clean:
#	-$(RM) -r output
	-$(RM) tempRegressionResults input/*.epx input/*.plx
