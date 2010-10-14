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

In t2-cmd.plx, none of the following variables ever update:

  MasterPlan: str
  Right: ret

The variable

  MasterPlan: return

updates to the value "one" after the assignment made on line 22.

The behavior of t1-sim.plx is described in t1-readme.txt.
