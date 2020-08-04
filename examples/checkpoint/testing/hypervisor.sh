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

# Preperatory work
plexilc plans/Test1.ple
plexilc plans/Test2.ple
make

# Spawn CPUS/2 copies of the tester
CPUS=$(nproc --all)
PROCESSES=$((CPUS/2))
# minimum of 1 process
PROCESSES=$([ "1" -ge $PROCESSES ] && echo "1" || echo "$PROCESSES")
echo "Running $PROCESSES procesees in parallel for $1 iterations"
echo "All errors will be logged to ./log.txt"
i="1"

while [ $i -le $PROCESSES ]
do
    ./run_tests.sh "$i" "$1" &
    ((i++))
done


# Wait for all subprocesees to finish
for job in `jobs -p`
do
    wait $job
done

echo "Appending logs"

i="1"
# Combine logs, ignoring errors if they don't exist
while [ $i -le $PROCESSES ]
do
    (cat "log-${i}.txt" >> log.txt  2>/dev/null)
    (rm "log-${i}.txt"  2>/dev/null)
    ((i++))
done


echo "Done"

