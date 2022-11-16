#! /bin/sh
# Install the Python virtual environment for the validator

# Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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
    cat <<EOF
Usage: $(basename "$0") [ --with-python <python_exe> ] [ --upgrade ]
python_exe defaults to $(command -v python3)
Requires Python 3.5 or newer.
EOF
}

python_exe=
do_upgrade=

while [ -n "$1" ]
do
    case "$1" in
        --with-python)
            shift
            if [ ! -x "$1" ]
            then
                echo "$(basename "$0"): Error: --with-python argument $1 is not executable" >&2
                exit 2
            fi
            python_exe="$1"
            ;;

        --upgrade)
            do_upgrade="$1"
            ;;

        -h | --help | help)
            usage
            exit 0
            ;;

        *)
            echo "$(basename "$0"): Error: unrecognized argument '$1'" >&2
            usage >&2
            exit 2
            ;;
    esac
    shift
done

here="$( cd "$(dirname "$0")" && pwd -P )"
venv_dir="$here/.venv"
our_pip="$venv_dir/bin/pip"
activate_script="$venv_dir/bin/activate"

# Get Python from environment, if supplied
# Prefer python3 if not
python_exe=${python_exe:-"$(command -v python3)"}

if [ -z "$python_exe" ]
then
    echo 'Error: no Python executable found or supplied.' >&2
    echo 'Please specify a Python 3.5 (or newer) interpreter.' >&2
    exit 2
elif [ ! -x "$python_exe" ]
then
    echo "Error: $python_exe is not executable." >&2
    echo 'Try again with a Python 3.5 (or newer) interpreter.' >&2
    exit 2
fi

bootstrap_venv()
{
    if ! "$python_exe" -c 'import venv' > /dev/null 2>&1
    then
        echo "$(basename "$0"): ERROR: Python module 'venv' is not installed." >&2
        echo 'Cannot set up virtual environment.' >&2
        return 1
    fi
    venv_options=''
    # Are pip and/or ensurepip installed locally?
    have_pip="$("$python_exe" -c 'import pip' > /dev/null 2>&1 && echo 'yes' || echo)"
    have_ensurepip="$("$python_exe" -c 'import ensurepip' > /dev/null 2>&1 && echo 'yes' || echo)"

    # DEBUG
    # echo "python_exe=$python_exe"
    # echo "have_pip=$have_pip"
    # echo "have_ensurepip=$have_ensurepip"

    if [ -z "$have_pip" ]
    then
        if [ -z "$have_ensurepip" ]
        then
            echo "$(basename "$0"): ERROR: Neither Python module 'pip' nor 'ensurepip' is installed." >&2
            echo 'Cannot set up virtual environment.' >&2
            return 1
        fi
        venv_options='--without-pip'
    elif [ -z "$have_ensurepip" ]
    then
        # Use the locally installed pip
        venv_options='--system-site-packages'
    fi

    echo 'Initializing Python virtual environment'
    if ! "$python_exe" -m venv "$venv_dir" "$venv_options"
    then
        echo 'Error: creating Python virtual environment failed.' >&2
        echo 'Check that your Python is version 3.5 or newer.' >&2
        return 1
    fi
}

#
# Start from clean if not upgrading
#

if [ -z "$do_upgrade" ] && [ -d "$venv_dir" ]
then
    echo 'Deleting existing virtual environment'
    rm -rf "$venv_dir" "$here/environment" bin include lib pyvenv.cfg
fi

if [ ! -d "$venv_dir" ]
then
    if ! bootstrap_venv
    then
        exit 1
    fi
elif [ -n "$do_upgrade" ]
then
    echo 'Upgrading Python virtual environment'
    if ! "$python_exe" -m venv "$venv_dir" "$do_upgrade"
    then
        echo 'Error: upgrading Python virtual environment failed.' >&2
        exit 1
    fi
fi

if [ ! -r "$activate_script" ]
then
    echo 'Error: Python virtual environment setup failed.' >&2
    exit 1
fi

# shellcheck source=.venv/bin/activate
( . "$activate_script" && \
      "$our_pip" install -q -U pip && \
      "$our_pip" install -q${do_upgrade:+ $do_upgrade} -r "$here/requirements.txt" )

# Write out environment for validate script
cat <<EOF > "$here/environment"
# Generated by setup.sh; do not edit
python_exe='$venv_dir/bin/python'
activate_script='$venv_dir/bin/activate'
EOF

exit 0
