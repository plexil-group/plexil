#! /usr/bin/perl

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

use strict;
use warnings;

my $fileout = 'RegressionResults';
my $testfile = $ARGV[0];
my $validfile = $ARGV[1];

my $encoding = ":encoding(ISO-8859-1)"; # ??

my $exitstatus = 0;

open(DIF, ">> $encoding", $fileout); # Open for append

my $testName = $testfile;
$testName =~ s/output\/RUN_(.*)_(.*)\.out/$1 + $2/;

sub stripline {
    my $line = shift(@_);
    $line =~ s/(0x[0-9a-fA-F]{2,16})//g;
    return $line;
}

# this is for comparing .out with .valid
# only if a .valid file is passed in
if ($validfile) {
    open(TEST, "< $encoding", $testfile);
    open(VALID, "< $encoding", $validfile);
    my @lines1 = <TEST>;
    my @lines2 = <VALID>;
    my $n = 0;
    my $m = 0;
    # FIXME: case where valid file has more lines than test
    # FIXME: case where test file has more lines than valid
    foreach my $validline (@lines2) {
        $validline = stripline($validline);
        my $testline = stripline($lines1[$n]);
        if ($testline ne $validline) {
            if ($m == 0) {
                print DIF "\n";
                print DIF "**** TEST FAILED: ", $testName; 
                print STDERR "\nTEST FAILED:  ", $testName;
                print STDERR "***\n  ------ .out file differs from .valid file\n";
            }
            $m++;
            print DIF "\nLine: ", $n;
            print DIF "\n";
            print DIF ".out:   ", $testline; 
            print DIF ".valid: ", $validline;  
            print DIF "\n";
        }  
        $n++;
    }
    if ($n == 0) {
        print DIF "\n";
        print DIF "\n**** TEST FAILED: ", $testName; 
        print DIF "***\n  ------ No .output file was provided\n";
        print STDERR "\nTEST FAILED:  ", $testName, "\n";
        print STDERR "  ------ No .output file was provided\n";
    } elsif ($m != 0) {
        $exitstatus = 1;
    }
    close(VALID);
    close(TEST);
} else {
    # Check for success of root node
    # for files where this enough to show success
    open(TEST, "< $encoding", $testfile);

    my @lines1 = <TEST>;
    my $n = 0;
    my $k = 0;
    foreach my $line1 (@lines1) {
        if ($line1 =~ m/Added plan:/ ){
            if ($lines1[$n+1] =~ m/^(.*)\{$/) {
                my $rootNode = $1;
                my $count = @lines1;
                for (my $i = $count-1; $i > 0; $i--) {
                    if($lines1[$i] =~ m/PlexilExec:printPlan/) {
                        $k = 1;
                        if ($lines1[$i+1] =~ m/^$rootNode\{$/) {
                            if ($lines1[$i+3] =~ m/^ Outcome: ([A-Z]*)$/) {
                                my $outcome = $1;
                                if ($outcome eq "SUCCESS") {
                                    $i = 0;
                                } else {
                                    print DIF "\n";
                                    print DIF "\n*** TEST FAILED:  ", $testName, " ***\n  ------- root node outcome = ", $outcome, "\n";
                                    print STDERR "\nTEST FAILED:  ", $testName, "\n  ------- root node outcome = ", $outcome, "\n";
                                    $i = 0;
                                    $exitstatus = 1;
                                }
                            } else {
                                print DIF "\n";
                                print DIF "*** TEST FAILED:  ", $testName, " ***\n  ------- root node outcome != SUCCESS\n";
                                print STDERR "\nTEST FAILED:  ", $testName, "\n  ------- root node outcome != SUCCESS\n";
                                $i = 0;
                                $exitstatus = 1;
                            }
                        }
                    }
                }       
            } 
        }
        $n++;
    }
    if ($k == 0) {
        print DIF "\n";
        print DIF "*** TEST FAILED:  ", $testName, " ***\n  ------- ERROR in test execution\n";
        print STDERR "\nTEST FAILED:  ", $testName, "\n  ------- ERROR in test execution\n";
    }
    close(TEST);
}

# this doesn't work (tempName) with libraries yet
my $fileErrs = 'tempRegressionResults';
open(F3, "< $encoding", $fileErrs);
my $i = 0;
my @lines3 = <F3>;
foreach my $line3 (@lines3) {
    $i++;
    if ($line3 =~ s/^RUN_exec-test-runner_g_rt\.-s\.(.*?)\.xml\.-p\.(.*?)\.xml.*/$2 + $1/) {
        chomp(my $tempName = $line3);
        if ($tempName eq $testName) {
            my $line3 = $lines3[$i++];
            while ($line3 && !($line3 =~ m/^RUN_exec-test-runner_g_rt\.-s\.(.*?)\.xml\.-p\.(.*?)\.xml.*/)) {
                print DIF $line3;
                $line3 = $lines3[$i++];
            }
        }
    }
}
close(F3);
close(DIF);

exit $exitstatus;
