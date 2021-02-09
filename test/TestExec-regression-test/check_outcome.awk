#! /usr/bin/awk -f
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

# Check output of a regression test plan for success of root node
# for plans where this is sufficient to show success

BEGIN {
# File name is expected to be output/<testname>.out
    testname = ARGV[1]
    sub("^output/", "", testname)
    sub("\.out$", "", testname)
    DIF = "RegressionResults"
    STDERR = "/dev/stderr"
    rootnode = ""
    laststate = ""
    outcome = ""
    error = ""
}

# The root node is at the start of every plan printout
/^[A-Za-z0-9_-]+{$/ {
    if (rootnode == "") {
        rootnode = $0
        sub("{$", "", rootnode)
    }
}

# This range matches one plan printout
/^[A-Za-z0-9_-]+{$/, /^}$/ {
    if ($0 ~ /^ State:/) {
        laststate = $2
    }
    if ($0 ~ /^ Outcome:/) {
        outcome = $2
    }
}

/^ERROR:/ {
    error = $0
}

END {
    if (error != "") {
        print "\n\n*** TEST FAILED: ", testname, "***\n  -------", error, "\n" >> DIF
        print "\nTEST FAILED: ", testname, "\n  -------", error, "\n" > STDERR
        exit 1
    }
    if (outcome == "SUCCESS") {
        exit 0
    }
    if (outcome != "") {
        print "\n\n*** TEST FAILED: ", testname, "***\n  ------- root node outcome =", outcome, "\n" >> DIF
        print "\nTEST FAILED: ", testname, "\n  ------- root node outcome =", outcome, "\n" > STDERR
        exit 1
    }
    if (rootnode != "") {
        print "\n\n*** TEST FAILED: ", testname, "***\n  ------- last root node state =", laststate, "\n" >> DIF
        print "\nTEST FAILED: ", testname, "\n  ------- last root node state =", laststate, "\n" > STDERR
        exit 1
    }
    print "\n\n*** TEST FAILED: ", testname, "***\n  ------- unrecognized output format\n" >> DIF
    print "\nTEST FAILED: ", testname, "\n  ------- unrecognized output format\n" > STDERR
    exit 1
}
