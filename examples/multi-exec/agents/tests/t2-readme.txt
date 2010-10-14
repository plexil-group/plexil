File: $SVNROOT/examples/multi-exec/agents/tests/t2-readme.txt
Date: Thu Oct 14 09:34:42 2010

In t2-cmd.pli (using t1-sim.pli), Plexil Viewer doesn't update
"return" variable in "MasterPlan" node for the first assignment (line
19), then updates the second assignment (line 22) with the value
("one") for the first assignment.

% cd plexil/examples/multi-exec/agents/test
% make
% xterm ipc &
% xterm -v -p t1-sim.plx -c sim.xml
% plexilexec -v -b -p t2-cmd.plx -c cmd.xml

Watch "return" variable in "MasterPlan" -- it updates only after the
second assignment, but to the wrong value
