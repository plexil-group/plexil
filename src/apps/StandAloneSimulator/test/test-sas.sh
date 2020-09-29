#! /bin/sh
# Run the SAS test

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/bin

set -e

if [ -z "$PLEXIL_HOME" ]
then
    echo "Error: Unable to set up Plexil environment."
    echo "Please set environment variable PLEXIL_HOME"
    echo "to the full pathname of your PLEXIL installation directory."
    return 1
fi

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
