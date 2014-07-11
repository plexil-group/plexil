#! /usr/bin/env bash

export PLEXIL_HOME=/Users/chucko/src/plexil-3-new-exp
# Add to this as needed
export DYLD_LIBRARY_PATH=$PLEXIL_HOME/src/third-party/pugixml/src/.libs:$PLEXIL_HOME/src/utils/.libs:$PLEXIL_HOME/src/expr/.libs:$PLEXIL_HOME/src/intfc/.libs:$PLEXIL_HOME/src/exec/.libs:$PLEXIL_HOME/src/interfaces/PlanDebugListener/.libs:$PLEXIL_HOME/src/interfaces/LuvListener/.libs:$PLEXIL_HOME/src/interfaces/Sockets/.libs:$PLEXIL_HOME/src/apps/TestExec/.libs
