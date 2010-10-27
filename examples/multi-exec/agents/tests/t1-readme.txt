File: $SVNROOT/examples/multi-exec/agents/tests/t1-readme.txt
Date: Tue Oct 12 17:25:00 2010

Plexil Viewer doesn't update "return" variable in "MasterPlan" node
in t1-cmd.pli.

% cd plexil/examples/multi-exec/agents/test
% make
% xterm ipc &
% xterm -v -p t1-sim.plx -c sim.xml
% plexilexec -v -b -p t1-cmd.plx -c cmd.xml

or

% cd plexil/examples/multi-exec/agents/test
% run-agents t1-sim t1-cmd -v

In t1-cmd.plx, none of the following variable ever update:

  MasterPlan: return
  Right: ret
  One: ret
  Two: ret

In t1-sim.plx, none of the following variables ever update:

  OnCommand_1: modX, ep2cp_hdl
  OnCommand_2: int, ep2cp_hdl
  OnMessage_3: ep2cp_hdl
