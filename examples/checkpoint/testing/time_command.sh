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

# Mac doesn't support sub-second time
if [[ "$OSTYPE" == "darwin"* ]];
then    
    time_s=$({ time $@ >/dev/null 2>/dev/null ; } 2>&1)
    time_s=$(echo "$time_s" | grep "real"  | awk '{print $2}' )
    minutes=$(echo "$time_s" | awk -F'm' '{print $1}')
    seconds=$(echo "$time_s" | awk -F'm' '{print $2}' | awk -F's' '{print $1}')
    nanoseconds=$(echo "$minutes $seconds" | awk '{seconds = $1 * 60 + $2 ; nanoseconds = seconds * 1000000000 
                                                                            print nanoseconds}')
    echo $nanoseconds
else
    # Begin timing
    ts=$(date +%s%N)
    # Run command and hide output
    $@  > /dev/null 2>&1
    # Finish timing and return the time as an integer number of nanoseconds
    echo $((($(date +%s%N) - $ts)))
fi
