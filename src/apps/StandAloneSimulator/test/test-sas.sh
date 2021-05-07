#! /bin/sh -e
# Run the SAS test

guess_plexil_home()
{
    # This test script is expected to be in $PLEXIL_HOME/src/apps/StandAloneSimulator/test
    TEST_DIR="$( cd "$(dirname "$(command -v "$0")")" && pwd -P )"
    cd "$TEST_DIR/../../../.." && pwd -P
}

if [ -z "$PLEXIL_HOME" ]
then
    PLEXIL_HOME="$(guess_plexil_home)"
    export PLEXIL_HOME
    echo "Defaulting PLEXIL_HOME to $PLEXIL_HOME"
fi

if [ ! -x "$PLEXIL_HOME/scripts/plexil-setup.sh" ]
then
    echo 'Error: Environment variable PLEXIL_HOME is set incorrectly.' >&2
    echo 'Please set it to the full pathname of the PLEXIL source directory' >&2
    echo " (presumably $(guess_plexil_home))" >&2
    exit 1
fi

export PLEXIL_HOME

. "$PLEXIL_HOME"/scripts/plexil-setup.sh

PLAN_SRC='sas-test.ple'
PLAN_PLX="$(basename "$PLAN_SRC" .ple).plx"

if [ ! -e "$PLAN_PLX" ]
then
    if [ -r "$PLAN_SRC" ]
    then
        plexilc "$PLAN_SRC"
    else
        echo "Error: plan source file $PLAN_SRC not found." >&2
        exit 1
    fi
fi

# Variables to hold process IDs
CENTRAL_PID=
SIM_PID=

cleanup()
{
    if [ -n "$SIM_PID" ]
    then
        kill "$SIM_PID"
    fi
    sleep 1
    if [ -n "$CENTRAL_PID" ]
    then
        kill "$CENTRAL_PID"
    fi
}

# Kill background processes on exit
trap cleanup EXIT HUP QUIT TERM

# Start IPC central
central -us >& /dev/null &
CENTRAL_PID=$!

sleep 1

# Start the simulator
run-sas unified-script.txt &
SIM_PID=$!

sleep 1

# Start the exec
universalExec -c config.xml -p sas-test.plx
