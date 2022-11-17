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
Usage: $(basename "$0") [ --with-python <python_exe> ] [ --reinstall | -U | --upgrade ]
python_exe defaults to $(command -v python3)
Requires Python 3.5 or newer.
EOF
}

do_reinstall=
do_upgrade=
python_exe=
user_python=

while [ -n "$1" ]
do
    case "$1" in

        -h | --help | help)
            usage
            exit 0
            ;;

        --reinstall)
            do_reinstall="$1"
            ;;

        -U | --upgrade)
            do_upgrade="$1"
            ;;

        --with-python)
            if [ ! -x "$2" ]
            then
                echo "$(basename "$0"): Error: --with-python argument $2 is not executable" >&2
                exit 2
            fi
            python_exe="$2"
            user_python='yes'
            shift
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
# defaults
venv_dir="$here/.venv"
our_pip="$venv_dir/bin/pip"
activate_script="$venv_dir/bin/activate"

# Default to Python 3 from $PATH
python_exe=${python_exe:-"$(command -v python3)"}

echo 'got python_exe'

if [ -z "$python_exe" ]
then
    echo "$(basename "$0"): Error: no Python executable found or supplied." >&2
    echo 'Please select a Python 3.5 (or newer) interpreter with the --use-python option.' >&2
    exit 2
elif [ ! -x "$python_exe" ]
then
    echo "$(basename "$0"): Error: $python_exe is not executable." >&2
    echo 'Please select a Python 3.5 (or newer) interpreter with the --use-python option.' >&2
    exit 2
fi

echo 'checked python_exe'

# Take inventory of Python modules for virtual environments.
have_venv="$("$python_exe" -c 'import venv' > /dev/null 2>&1 && echo 'yes' || echo)"
have_virtualenv="$(command -v virtualenv || echo '')"

# DEBUG
# echo "python_exe=$python_exe"
# echo "have_venv=$have_venv"
# echo "have_virtualenv=$have_virtualenv"
# echo "have_pip=$have_pip"
# echo "have_ensurepip=$have_ensurepip"

virtual_env=
virtual_env_options=

if [ -n "$have_virtualenv" ]
then
    # Prefer virtualenv
    virtual_env='virtualenv'
    if [ -n "$user_python" ]
    then
        virtual_env_options="--python=${python_exe}"
    fi
elif [ -n "$have_venv" ]
then
    virtual_env="$python_exe -m venv"
else
    echo "$(basename "$0"): Error: neither 'venv' nor 'virtualenv' available for $python_exe" >&2
    echo 'Please select a different Python 3 interpreter with the --use-python option.' >&2
    exit 1
fi

bootstrap_virtual_environment()
{
    echo 'Initializing Python virtual environment'
    local have_ensurepip=
    local install_pip=
    if [ -z "$have_virtualenv" ]
    then
        have_ensurepip="$("$python_exe" -c 'import ensurepip' > /dev/null 2>&1 && echo 'yes' || echo)"
        if [ -z "$have_ensurepip" ]
        then
            virtual_env_options='--without-pip'
            install_pip='yes'
        fi
    fi

    # DEBUG
    echo "${virtual_env}${venv_options:+ $venv_options} $venv_dir"
    if ! ${virtual_env}${venv_options:+ $venv_options} "$venv_dir"
    then
        echo "$(basename "$0"): Error: virtual environment creation failed." >&2
        return 1
    fi

    if [ -n "$install_pip" ]
    then
        echo 'Installing pip in the virtual environment'
        if ! curl https://bootstrap.pypa.io/get-pip.py -s -S -o "${venv_dir}/bin/get-pip.py" && \
                ( . "${venv_dir}/bin/activate" && "${venv_dir}/bin/python3" "${venv_dir}/bin/get-pip.py" )
        then
            echo "$(basename "$0"): Error: Created virtual environment, but failed to install 'pip' in it." >&2
            return 1
        fi
    fi

    # Do initial software installation
    echo 'Installing required modules in the virtual environment'
    if ! ( . "$activate_script" && \
               "$our_pip" install -q -U pip && \
               "$our_pip" install -q -r "$here/requirements.txt" )
    then
        echo "$(basename "$0"): Error: Failed to install required modules in the virtual environment." >&2
        return 1
    fi

    # Write out environment for validate script
    cat <<EOF > "$here/environment"
# Generated by setup.sh; do not edit
python_exe='$venv_dir/bin/python'
activate_script='$venv_dir/bin/activate'
EOF

    echo "Virtual environment successfully created in $venv_dir"
    return 0
}

upgrade_virtual_environment()
{
    local upgrade_options=
    if [ -n "$have_virtualenv" ]
    then
        upgrade_options=''
    else
        upgrade_options='--upgrade'
    fi
    echo 'Upgrading Python virtual environment'
    if ! ${virtual_env} ${upgrade_options} "$venv_dir"
    then
        echo "$(basename "$0"): Error: Failed to upgrade virtual environment." >&2
        return 1
    fi
    echo 'Upgrading installed modules in the virtual environment'
    if ! ( . "$activate_script" && \
               "$our_pip" install -q -r "$here/requirements.txt" )
    then
        echo "$(basename "$0"): Error: Failed to upgrade modules in the virtual environment." >&2
        return 1
    fi

    echo "Virtual environment successfully upgraded in $venv_dir"
    return 0
}

if [ -d "$venv_dir" ] && [ -n "$do_reinstall" ]
then
    echo 'Deleting existing virtual environment'
    rm -rf "$venv_dir" "$here/environment"
fi

if [ ! -d "$venv_dir" ] || [ ! -f "$here/environment" ]
then
    if ! bootstrap_virtual_environment
    then
        exit 1
    fi
else
    # if [ -z "do_upgrade" ]
    # then
    #     # Check if an upgrade is required
    #     # TODO
    # fi

    if [ -n "$do_upgrade" ]
    then
        if ! upgrade_virtual_environment
        then
            echo 'Re-run this command with the --reinstall option.' >&2
            exit 1
        fi
    fi
fi

exit 0
