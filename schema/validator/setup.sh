#! /bin/sh
# Install the Python virtual environment for the validator

# Copyright (c) 2006-2020, Universities Space Research Association (USRA).
#  All rights reserved.
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

set -e

usage()
{
    echo "Usage: $(basename "$0") [ --with-python <PYTHON> ] [ --upgrade ]"
    echo "PYTHON defaults to $(command -v python3)"
    echo 'Requires Python 3.5 or newer'
}

UPGRADE=

while [ -n "$1" ]
do
    case "$1" in
        --with-python)
            shift
            PYTHON="$1"
            ;;

        --upgrade)
            UPGRADE="$1"
            ;;

        -h | --help | help)
            usage
            exit 0
            ;;

        *)
            echo "Error: argument '$1' is not understood" >&2
            usage >&2
            exit 2
            ;;
    esac
    shift
done

# Get Python from environment, if supplied
# Prefer python3 if not
PYTHON=${PYTHON:-"$(command -v python3)"}

if [ -z "$PYTHON" ]
then
    echo 'Error: no Python executable found or supplied.' >&2
    echo 'Please specify a Python 3.5 (or newer) interpreter.' >&2
    exit 2
elif [ ! -x "$PYTHON" ]
then
    echo "Error: $PYTHON is not executable." >&2
    echo 'Try again with a Python 3.5 (or newer) interpreter.' >&2
    exit 2
fi

HERE="$( cd "$(dirname "$0")" ; pwd )"
VENV_DIR="$HERE/.venv"
PIP="$VENV_DIR/bin/pip"
ACTIVATE="$VENV_DIR/bin/activate"

#
# Start from clean if not upgrading
#

if [ -z "$UPGRADE" ] && [ -d "$VENV_DIR" ]
then
    echo 'Deleting existing virtual environment'
    rm -rf "$VENV_DIR" "$HERE/environment"
fi

if [ ! -d "$VENV_DIR" ]
then
    echo 'Initializing Python virtual environment'
    if ! "$PYTHON" -m venv "$VENV_DIR"
    then
        echo 'Error: creating Python virtual environment failed.' >&2
        echo 'Check that your Python is version 3.5 or newer.' >&2
        exit 2
    fi
elif [ -n "$UPGRADE" ]
then
    echo 'Upgrading Python virtual environment'
    if ! "$PYTHON" -m venv $UPGRADE "$VENV_DIR"
    then
        echo 'Error: upgrading Python virtual environment failed.' >&2
        exit 2
    fi
fi

if [ ! -r "$ACTIVATE" ]
then
    echo 'Error: Python virtual environment setup failed.' >&2
    exit 1
fi

( . "$ACTIVATE" && \
      "$PIP" install -q -U pip && \
      "$PIP" install -q $UPGRADE -r "$HERE/requirements.txt" )

# Write out environment for validate script
cat << EOF > "$HERE/environment"
# Generated by setup.sh; do not edit
PYTHON='$VENV_DIR/bin/python'
ACTIVATE='$VENV_DIR/bin/activate'
EOF

exit 0
