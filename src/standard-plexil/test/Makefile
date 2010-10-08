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

# Translates Standard PLEXIL test files into Core PLEXIL,
# placing the output in the 'output' subdirectory, and
# verifies that the output matches the validated version of
# the file.

SHELL := /bin/sh

TEST_PLANS := $(basename $(wildcard *.ple))

TEST_SCRIPTS = $(basename $(wildcard *.pst))

# Temporary file
DIFF := /tmp/plexil-$$$$

default: plans scripts

plans:
	@ echo Translating Standard Plexil test plans...
	@ for i in $(TEST_PLANS); do \
	        plexilc -q -o output/$$i.plx $$i.ple  ; \
		diff -b output/$$i.plx output/valid-$$i.plx \
		  > $(DIFF) ; \
		diffsize=`wc $(DIFF) | awk '{print $$1}'` ; \
		if [ $$diffsize -eq 0 ] ; then \
		  echo "     Verified $$i.plx" ; \
		else \
		  echo WARNING: $$i.plx does not verify! ; \
		fi ; \
	  done
	@ echo Finished.

scripts:
	@ echo Translating Standard Plexil test scripts...
	@ for i in $(TEST_SCRIPTS); do \
	        plexilscript $$i.pst  ; \
		if [ -e $$i.psx ] ; then \
			mv $$i.psx output ; \
			diff output/$$i.psx output/valid-$$i.psx > $(DIFF) ; \
			diffsize=`wc $(DIFF) | awk '{print $$1}'` ; \
			if [ $$diffsize -eq 0 ] ; then \
		  	  echo "     Verified $$i.psx" ; \
			else \
		  	  echo WARNING: $$i.psx does not verify! ; \
			fi ; \
		fi ; \
	  done
	@ echo Finished.

clean:
	@ for i in $(TEST_PLANS); do \
	        rm -f output/$$i.plx output/$$i.epx  ; \
	  done
	@ for i in $(TEST_SCRIPTS); do \
	        rm -f output/$$i.psx ; \
	  done

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

