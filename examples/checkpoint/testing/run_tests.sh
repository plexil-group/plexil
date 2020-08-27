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

if [ "$#" -ne 2 ]
then
  echo "Usage: $0 ARBITRARY_INTEGER NUMBER_OF_TESTS" >&2
  exit 1
fi

TIMEOUT_COMMAND=""
if [[ -n $(command -v timeout) ]]
then
    TIMEOUT_COMMAND="timeout"
elif [[ -n $(command -v gtimeout) ]]
then
    TIMEOUT_COMMAND="gtimeout"
else
    echo "Cannot find timeout or gtimeout, exiting"
fi


if [[ -n "$TIMEOUT_COMMAND" ]]
then
   # Generate our own configurtion file with an independent saves directory
   sed "s/saves/saves-${1}/g" interface-config.xml > "interface-config-${1}.xml"

   # Make sure our save directory is fresh and exists
   rm -r "./saves-$1" 2>/dev/null
   mkdir "./saves-$1" 2>/dev/null

   # Figure out how long a run takes, this is our range for when to kill it
   TIME=$(./time_command.sh plexilexec -p plans/Test1.plx -c "interface-config-${1}.xml")
   MILLI=$(bc <<< "scale=2; $TIME/1000000")
   echo "Test run took $MILLI ms, using 1.5x that as an upper bound"

   # Run the test plan with random timeout and analyze the results
   i=1

   while [ "$i" -le "$2" ]
   do
       # Uses a 61-bit range to (nearly) randomly generate a number between 1 and 1.5*TIME
       TIMEOUT=$(( $((RANDOM|(RANDOM<<15)|(RANDOM<<30)| ((RANDOM<<45)^(RANDOM<<48)) )) \
		       % (TIME+(TIME>>1)) + 1 ))

       # Run the test with the randomly generated timeout
       ./single_test.sh "$1" "$TIMEOUT" "$TIMEOUT_COMMAND"
       i=$[$i+1]
   done
fi
# Cleanup
rm "interface-config-${1}.xml"

echo "Process $1 done"
