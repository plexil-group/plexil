File: $SVNROOT/examples/multi-exec/agents/tests/t4-readme.txt
Date: Mon Nov  1 19:26:26 2010

In t4-sim.pli, which exactly the same as t1-sim.pli except for the
renaming of "modX" to "state", the renamed variable causes an obscure
error:

  ERROR: Expression.cc:190: Value 5 invalid for (id_626[au](EXECUTING): state(EXECUTING))
  Assertion failed: (false), function handleAssert, file Error.cc, line 89.

% cd plexil/examples/multi-exec/agents/tests
% make
% xterm ipc &
% xterm -v -p t4-sim.plx -c sim.xml
% plexilexec -v -b -p t1-cmd.plx -c cmd.xml

or

% cd plexil/examples/multi-exec/agents/tests
% run-agents [-v] t4-sim t1-cmd

The run ends  with the error quoted above.  However,

% run-agents [-v] t1-sim t1-cmd

executes fine.
