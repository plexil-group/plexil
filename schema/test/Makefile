# Copyright (c) 2006-2010, Universities Space Research Association (USRA).
#  All rights reserved.
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
# output in the 'translated' subdirectory.

SHELL = /bin/sh

TESTS = UncheckedSequence1 UncheckedSequence2 \
	UncheckedSequence3 UncheckedSequence4 \
	Sequence1 Sequence2 Sequence3 Sequence4 \
	Concurrence1 Concurrence2 Concurrence3 Concurrence4 Concurrence5 \
	Try1 Try2 Try3 Try4 If1 If2 If3  While1 While2 While3 For1 For2 For3 \
	NodeCheck1 MessageReceived1 Sequence-Concurrence OnMessage OnCommand \
	Lookup1 Lookup2

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
