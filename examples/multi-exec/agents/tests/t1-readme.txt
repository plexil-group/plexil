File: $SVNROOT/examples/multi-exec/agents/tests/t1-readme.txt
Date: Tue Oct 12 17:25:00 2010

Plexil Viewer doesn't update "return" variable in "MasterPlan" node
in t1-cmd.pli.

% cd plexil/examples/multi-exec/agents/test
% make
% xterm ipc &
% xterm -v -p t1-sim.plx -c sim.xml
% plexilexec -v -b -p t1-cmd.plx -c cmd.xml

Watch "return" variable in "MasterPlan" -- it never changes (10/12/10)
