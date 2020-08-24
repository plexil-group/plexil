#!/bin/bash
# Copyright (c) 2020-2020, Universities Space Research Association (USRA).
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

# Helper function for floating point arithmetic
calc(){ awk "BEGIN { print "$*" }"; }

# Make sure our save directory is fresh and exists
(rm -r "./saves-$1" 2>/dev/null)
(mkdir "./saves-$1" 2>/dev/null)


# Run our plan with a timeout, killing when time expires (-k 0)
# Then run our analysis plan

echo "Process $1: Terminating after  $(calc "$2"/1000000 ) ms"
# Cut from "--START" to just before "Plan complete"
# Then replace spaces with % for passing to ParseTest
# Then append "PRESTART|" to guarantee 2 arguments to ParseTest
arg1=$("$3" -k 0 $(calc "$2"/1000000000 ) plexilexec -p plans/Test1.plx -c "interface-config-${1}.xml"   \
	    | grep -o -e "---START.*"  \
            | sed -E 's/(Plan.*)|(Killed.*)//'  \
	    | sed 's/ /%/g')
arg1="PRESTART|${arg1}"

arg2=$(             plexilexec -p plans/Test2.plx -c "interface-config-${1}.xml"  \
	    | grep -o -e "---START.*"  \
	    | sed -E 's/(Plan.*)|(Killed.*)//'  \
	    | sed 's/ /%/g')
arg2="PRESTART|${arg2}"

# Run our c analysis script
set -o pipefail
output=$(./ParseTest $arg1 $arg2)
error=$?

# If there was an error code, log it
if [ "$error" -ne "0" ]
then
    echo "-------------------------------------" >> "log-${1}.txt"
    echo "First output:" >> "log-${1}.txt"
    echo "$arg1" >> "log-${1}.txt"
    echo "Second output:" >> "log-${1}.txt"
    echo "$arg2" >> "log-${1}.txt"
    echo "Error:" >> "log-${1}.txt"
    echo "$error" >> "log-${1}.txt"
    echo "$output" >> "log-${1}.txt"
    echo "     " >> "log-${1}.txt"
    cd "saves-${1}"
    cat $(ls -t) >> "../log-${1}.txt"  2>/dev/null
    cd ..
fi

(rm -r "./saves-$1" 2>/dev/null)
