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

PLEXIL_HOME			?= ../../../..

VALID_TESTS	:= array-add array-complex \
 basic-wait-test \
 commandSequence1 concurrenceOfAssignments \
 emptyNodeWithVariables \
 ifWithClauseIds Increment interface-test \
 literal-format-test \
 minimal minimalCommand minimalCommandWithNodeId minimalWithNodeId \
 on-command-test on-message-test \
 SafeDrive sequenceOfAssignments simpleAssignment simpleIf simpleIfThenElse sync-cmd-test

VALID_TEST_FILES := $(VALID_TESTS:%=input/%.ple)

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

syntax-error-tests:

semantic-error-tests:

output:
	-mkdir -p output

clean:
	-$(RM) -r output
	-$(RM) tempRegressionResults input/*.epx input/*.plx
