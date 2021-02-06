#! /usr/bin/env perl
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

use strict;
use warnings;

my $testfile = $ARGV[0];
if ( ! $testfile ) {
    print STDERR "ERROR: check_outcome.pl: Missing argument\n";
    print STDERR "Usage: perl check_outcome.pl <test.out>";
    exit 2;
}

my $encoding = ":encoding(ISO-8859-1)"; # ??

my $fileout = 'RegressionResults';
open(DIF, ">> $encoding", $fileout); # Open for append

my $exitstatus = 0;

my $testName = $testfile;
$testName =~ s|output/(.*)\.out|$1|;

if ( ! open(TEST, "< $encoding", $testfile) ) {
    print DIF "\n\n**** TEST FAILED: ", $testName, "***\n  ------ Unable to open output file\n";
    print STDERR "\nTEST FAILED:  ", $testName, "\n  ------ Unable to open output file\n";
    return 2;
}

# First line should start with '[PlexilExec:addPlan]Added plan:'
my $line = <TEST>;
if ( ! $line ) {
    prematureEOF();
} elsif ( ! $line =~ m/Added plan:/ ) {
    # print STDERR "\nERROR: Expecting \"Added plan:\"\n";
    wrongFormat();
}

# Next line will have root node ID followed by right curly brace
my $rootNode = '';
$line = <TEST>;
if ( ! $line ) {
    prematureEOF();
} elsif ( $line =~ m/^(.*)\{$/ ) {
    $rootNode = $1;
} else {
    # print STDERR "\nERROR: Expecting root node\n";
    wrongFormat();
}

# Scan forward to printPlan with outcome, if any
do {
    do {
        $line = <TEST>;            
        if ( ! $line ) {
            prematureEOF();
        }
    } while ( $line !~ m/PlexilExec:printPlan/ );

    # printPlan found, next line should be root node name
    $line = <TEST>;
    if ( ! $line ) {
        prematureEOF();
    } elsif ( $line !~ m/^$rootNode\{$/ ) {
        # print STDERR "\nERROR: Expecting root node\n";
        wrongFormat();
    }

    # Next line should be state
    $line = <TEST>;
    if ( ! $line ) {
        prematureEOF();
    } elsif ( $line !~ m/^ State:/ ) {
        # print STDERR "\nERROR: Expecting \"State:\"\n";
        wrongFormat();
    }
} while ( $line !~ m/^ State: FINISHED/ );

# Next line should be outcome
my $outcome = '';
$line = <TEST>;
if ( ! $line ) {
    prematureEOF();
} elsif ( $line =~ m/^ Outcome: ([A-Z]*)$/ ) {
    $outcome = $1;
} else { 
    # print STDERR "\nERROR: Expecting \"Outcome:\", got: \n", $line, "\n";
    wrongFormat();
}

if ( $outcome eq "SUCCESS" ) {
    $exitstatus = 0;
} else {
    print DIF "\n\n*** TEST FAILED:  ", $testName, " ***\n  ------- root node outcome = ", $outcome, "\n";
    print STDERR "\nTEST FAILED:  ", $testName, "\n  ------- root node outcome = ", $outcome, "\n";
    $exitstatus = 1;
}
close(TEST);
close(DIF);

exit $exitstatus;
