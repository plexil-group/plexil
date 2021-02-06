#! /bin/sh -e
# Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

# Compare test output with output from a known good run,
# ignoring pointer differences

if [ $# -ne 2 ]
then
    echo "Error: $(basename "$0") requires exactly two filenames" >&2
    echo "Usage: $(basename "$0") <test.out> <test.valid>" >&2
    exit 2
fi

SED_CMD='s/(0x[0-9a-fA-F]{2,16})//g'

TEST_NAME="$(basename "$1" '.out')"

TEST_OUT_TMP="$(mktemp 'test_out_XXXXXX')"
VALID_TMP="$(mktemp 'valid_XXXXXX')"

cleanup()
{
    trap - INT QUIT EXIT HUP KILL
    rm -f "$TEST_OUT_TMP" "$VALID_TMP"
}

trap cleanup INT QUIT EXIT HUP KILL

sed -E -e "$SED_CMD" < "$1" > "$TEST_OUT_TMP"
sed -E -e "$SED_CMD" < "$2" > "$VALID_TMP"
STATUS=0
if ! diff "$TEST_OUT_TMP" "$VALID_TMP" >> RegressionResults
then
    printf "\n\n**** TEST FAILED: %s ***\n  ------ .out file differs from .valid file\n\n" "$TEST_NAME" >> RegressionResults
    printf "\nTEST FAILED:  %s\n  ------ .out file differs from .valid file\n" "$TEST_NAME" >&2
    STATUS=1
fi

cleanup

exit $STATUS
