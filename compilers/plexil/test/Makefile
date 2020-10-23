# Makefile for Plexil compiler test suite

# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Universities Space Research Association nor the
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

SHELL = /bin/sh

# Check environment, in case the user has somehow forgotten to set PLEXIL_HOME

TOP_DIR := $(realpath $(join $(dir $(firstword $(MAKEFILE_LIST))),../../..))

ifeq ($(PLEXIL_HOME),)
PLEXIL_HOME := $(TOP_DIR)
else
ifneq ($(PLEXIL_HOME),$(TOP_DIR))
$(error Environment variable PLEXIL_HOME is in error. It must be set to $(TOP_DIR) before proceeding)
endif
endif

export PLEXIL_HOME

SMALL_TESTS = AddArray AnyArgs_empty_test ArglistArithmeticType ArrayEquality \
 ArrayLookup array-add array-complex array-copy array_size_test \
 assignment0 assignment1 assignment2 \
 basic-wait-test boolean1 boolean2 boolean3 boolean4 boolean5 \
 CallWithPostCondition \
 command0 command1 command2 command3 command4 command5 command-handle1 \
 comment1 ComputedCommandName concat1 concurrence0 concurrence1 concurrence2 \
 concurrence3 CopyReals CruiseControl \
 date1 Decrement Do1 Do2 Do3 \
 DriveToSchool DriveToTarget duplicate-node-name \
 empty0 empty1 empty2 empty3 empty4 empty5 empty6 empty7 empty8 \
 Exchange executing1 executing2 expr1 expr3 expr4 \
 failed1 Fibs Fib PrintFib FibTest finished1 for1 for2 \
 if2 if3 if4 ifWithClauseIds inactive1 Increment interface-test interface-test1 \
 invariantFail1 isKnown1 \
 iterationEnded1 iterationFailed1 iterationSucceeded1 \
 LibraryCall1 LibraryCall2 LibraryCall3 LibraryCall4 library-call1 \
 LibraryCallWithArray library-node1 literal-format-test \
 lookup2 LookupWithComputedStateName \
 maxTest minTest mod1 \
 NestedBlocks1 NestedBlocks2 NestedBlocks3 NestedBlocks4 NestedBlocks5 \
 NestedBlocks6 NestedBlocks7 NoChildFailedTest nodeOutcome1 nodeState1 nodeState2 \
 on-command1 on-command2 on-message1 on-message2 \
 preconditionFailed1 \
 resource0 Resource1 Resource2 Resource3 Resource4 \
 RoboSimSimple2 SafeDrive \
 sequence0 sequence1 sequence2 sequence3 \
 checked-sequence0 checked-sequence1 checked-sequence2 checked-sequence3 \
 unchecked-sequence0 unchecked-sequence1 unchecked-sequence2 unchecked-sequence3 \
 simpleIf SimpleDrive simpleIfThenElse simpleAssignment skipped1 \
 StringConcat1 StringConcat2 StringConcat3 StringConcat4 StringConcat5 \
 StringConcat6 StringConcat7 StringConcat8 strlen_test \
 succeeded1 Sum sync-cmd-test sync-cmd-test1 \
 test-recv test-send time-lookup-test time-lookup-test1 time_tolerance_test \
 timed-iteration tolerance1 try1 \
 UnaryMinus update1 update2 update3 \
 wait1 waiting1 While0 while1

SYNTAX_ERROR_TESTS   = array-library-call

SEMANTIC_ERROR_TESTS = BadArrayAdd BadArrayLookup badNodeNameTest BadResource0 \
 duplicateLookupDecl1 expr5 expr2 \
 lookup1 LibraryCall5 LibraryCall6 \
 on-message3 SeekGoalAndEnergy

TESTS	         := $(SMALL_TESTS)

# The VALID_TEST* macros are used by the original test framework.  

VALID_TESTS	:= $(TESTS)

VALID_TEST_FILES := $(VALID_TESTS:%=input/%.ple)

EPX_SCHEMA := $(PLEXIL_HOME)/schema/extended-plexil.xsd

all: test syntax-error-tests semantic-error-tests

setup:
	@rm -f tempRegressionResults ; touch tempRegressionResults

output:
	@mkdir -p output
	@$(RM) -f output/*.epx

test: | setup output
	@ echo Translating Standard Plexil tests...
	@ failure=0;\
	echo '--- Begin validation and verification tests ---' ; \
	for i in $(TESTS) ; \
    do \
		if ! ./compile-validate "$$i" ; \
	    then \
			failure=1 ; \
		fi; \
	done ; \
    if [ $${failure} -ne 0 ] ; then \
		echo '*** Validation tests failed ***' ; \
		exit $${failure} ; \
	fi ; \
	echo '--- Validation and verification tests complete ---'

syntax-error-tests: | setup output
	@ syntactic_failure=0; \
	echo '--- Begin syntax checking tests ---' ; \
	echo '--- Begin syntax checking tests ---' >> tempRegressionResults ; \
	for f in $(SYNTAX_ERROR_TESTS) ; do \
		echo $$f >> tempRegressionResults ; \
		if ../PlexilCompiler --epx-only -o output/$$f.epx input/$$f.ple 1>> tempRegressionResults 2>&1 ; \
		then syntactic_failure=1 ; echo $$f.ple: failed to detect expected syntax error ; fi ; \
	done ; \
	if [ $${syntactic_failure} -ne 0 ] ; then \
		echo '*** Syntax checking tests failed ***' ; \
		exit $${syntactic_failure} ; \
	fi ; \
	echo '--- Syntax error tests passed ---'

semantic-error-tests: | setup output
	@ semantic_failure=0; \
	echo '--- Begin semantic checking tests ---' ; \
	echo '--- Begin semantic checking tests ---' >> tempRegressionResults ; \
	for f in $(SEMANTIC_ERROR_TESTS) ; do \
		echo $$f >> tempRegressionResults ; \
		if ../PlexilCompiler --epx-only -o output/$$f.epx input/$$f.ple 1>> tempRegressionResults 2>&1 ; \
		then semantic_failure=1 ; echo $$f.ple: failed to detect expected semantic error ; fi ; \
	done ; \
	if [ $${semantic_failure} -ne 0 ] ; then \
		echo '*** Semantic checking tests failed ***' ; \
		exit $${semantic_failure} ; \
	fi ; \
	echo '--- Semantic checking tests passed ---'

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

clean:
	-@$(RM) tempRegressionResults input/*.epx input/*.plx output/*.epx

.PHONY: setup output test syntax-error-tests semantic-error-tests checkall passall clean
