File: $SVNROOT/examples/multi-exec/agents/tests/t1-readme.txt
Date: Tue Oct 12 17:25:00 2010

Plexil Viewer doesn't update the "return" variable in "MasterPlan"
node in t1-cmd.pli.

% cd plexil/examples/multi-exec/agents/test
% make
% xterm ipc &
% xterm -v -p t1-sim.plx -c sim.xml
% plexilexec -v -b -p t1-cmd.plx -c cmd.xml

or

% cd plexil/examples/multi-exec/agents/test
% run-agents -v t1-sim t1-cmd

In t1-cmd.plx, the following variable doesn't update:

  MasterPlan: return

