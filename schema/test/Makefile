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

# Translates Extended PLEXIL test files into Core PLEXIL, placing the
# output in the 'output' subdirectory, and compares them with the
# "gold standard" files in the 'verified' subdirectory.

SHELL = /bin/sh

SYNCH =	SynchronousCommand1 SynchronousCommand2 SynchronousCommand3 \
	SynchronousCommand4 SynchronousCommand5 SynchronousCommand6 \
	SynchronousCommand7 SynchronousCommand8 SynchronousCommand9 \
	SynchronousCommand10

REST = UncheckedSequence1 UncheckedSequence2 \
	UncheckedSequence3 UncheckedSequence4 \
	Sequence1 Sequence2 Sequence3 Sequence4 \
	CheckedSequence1 CheckedSequence2 \
	CheckedSequence3 CheckedSequence4 \
	Concurrence1 Concurrence2 Concurrence3 Concurrence4 Concurrence5 \
    If1 If2 If3 If4 If5 If6 If7 If8 If9 \
	While1 While2 While3 While4 While5 While6 \
    For1 For2 For3 For4 For5 \
	Do0 Do1 Do2 Do3 \
	Try1 Try2 Try3 Try4 \
	NodeCheck1 MessageReceived1 Sequence-Concurrence OnMessage OnCommand \
	OnCommandSequence OnCommandStartCondition OnCommandSkipCondition \
	Lookup1 Lookup2 Wait1 Wait2 Wait3 Interface

TESTS = $(SYNCH) $(REST)

PLEXIL_HOME ?= ../..

EPLEXIL = ../../scripts/eplexil

CHECKER = ./test-translation

LOG = verify.out

default: test

output:
	@ mkdir -p $@

test: output
	@ export PLEXIL_HOME="$(PLEXIL_HOME)"; \
	rm -f $(LOG) ; touch $(LOG) ; \
	test_failed=0; \
	for i in $(TESTS); do \
		if ! $(CHECKER) $$i >> $(LOG) ; then test_failed=1 ; fi ; \
	done ; \
	if [ $$test_failed -ne 0 ] ; then \
		echo "Test(s) failed. See $(LOG) for details." ; \
		exit 1 ; \
	fi 
	@ echo All files verified.

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
