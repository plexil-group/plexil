#! /bin/sh
# Install the Python virtual environment for the validator

# Copyright (c) 2006-2023, Universities Space Research Association (USRA).
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

here="$( cd "$(dirname "$0")" && pwd -P )"

# Minimum Python version required
py_ver_major=3
py_ver_minor=5

venv_dir="${here}/.venv"
venv_pip="${venv_dir}/bin/pip"
venv_python="${venv_dir}/bin/python"
activate_script="${venv_dir}/bin/activate"
get_pip_py="${venv_dir}/bin/get-pip.py"

usage()
{
    cat <<EOF
Usage: $(basename "$0") [ <option> ]*
Set up or update a Python virtual environment in $venv_dir 
Options:
 -h, -help, --help      Prints this message and exits.
 -q, --quiet            Print only essential output.
 -r, --reinstall        Delete the existing environment and reinstall.
 -U, --upgrade          Upgrade an existing environment.
 -v, --verbose          Describe what the script is doing in detail.
 --with-python <path>   Use <path> as the Python executable for initial setup.
                        Default (from \$PATH) is $(command -v python3)
Requires Python ${py_ver_major}.${py_ver_minor} or newer.
EOF
}

# Verbosity options
quiet=
verbose=

msg()
{
    test -n "$quiet" || echo "$@"
}

verbose_msg()
{
    test -z "$verbose" || echo "$@"
}

error_msg()
{
    echo "$(basename "$0"): Error:" "$@" >&2
}

usage_error()
{
    error_msg "$@"
    usage >&2
    exit 2
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

        -q | --quiet)
            if [ -n "$verbose" ]
            then
                usage_error "'$1' option conflicts with '$verbose'"
            fi
            quiet="$1"
            ;;


        -R | --reinstall)
            if [ -n "$do_upgrade" ]
            then
                usage_error "'$1' option conflicts with '$do_upgrade'"
            fi
            do_reinstall="$1"
            ;;

        -U | --upgrade)
            if [ -n "$do_reinstall" ]
            then
                usage_error "'$1' option conflicts with '$do_reinstall'"
            fi
            do_upgrade="$1"
            ;;

        -v | --verbose)
            if [ -n "$quiet" ]
            then
                usage_error "'$1' option conflicts with '$quiet'"
            fi
            verbose="$1"
            ;;

        --with-python)
            if [ $# -lt 2 ]
            then
                usage_error "Missing argument to '$1' option"
            fi
            python_exe="$2"
            user_python='yes'
            shift
            ;;

        *)
            usage_error "unrecognized argument '$1'"
            ;;
    esac
    shift
done

verbosity="${verbose}${quiet}"

# Default to Python 3 from $PATH if not supplied
# Don't exit the script if not found
python_exe=${python_exe:-"$(command -v python3)"} || true

check_python()
{
    if [ -z "$python_exe" ]
    then
        error_msg "no Python executable found or supplied."
        echo "Please use the --with-python option to specify an appropriate Python executable." >&2
        usage >&2
        return 2
    fi

    if [ ! -e "$python_exe" ]
    then
        # No such file. Maybe it's in $PATH?
        local maybe_python=
        if [ "$python_exe" = "${python_exe##*/}" ] # i.e. no / in $python_exe
        then
            if maybe_python="$(command -v "$python_exe" 2> /dev/null)"
            then
                python_exe="$maybe_python"
            fi
        fi
        if [ -z "$maybe_python" ]
        then
            error_msg "$python_exe not found."
            echo "Please use the --with-python option to specify an appropriate Python executable." >&2
            return 2
        fi
    fi
    
    if [ ! -x "$python_exe" ]
    then
        error_msg "$python_exe is not executable."
        echo "Please use the --with-python option to specify an appropriate Python executable." >&2
        return 2
    fi

    local version_script="import sys
sys.version_info.major >= ${py_ver_major} and sys.version_info.minor >= ${py_ver_minor} and print('OK')
"

    # Check that python_exe is an appropriate version.
    python_ok="$("$python_exe" -c "$version_script")"
    if [ -z "$python_ok" ]
    then
        error_msg "This script requires at least Python ${py_ver_major}.${py_ver_minor}."
        echo 'Please use the --with-python option to specify an appropriate Python executable.' >&2
        return 1
    fi
}

bootstrap_virtual_environment()
{
    check_python || return 1

    msg "Setting up virtual environment for $python_exe"

    # Select virtual environment tools
    # Prefer virtualenv if present
    local virtual_env=
    local virtual_env_options=
    local install_pip=
    if virtualenv="$(command -v virtualenv)" # status == 0 if found
    then
        msg "Using virtualenv at $virtualenv"
        virtual_env='virtualenv'
        if [ -n "$user_python" ]
        then
            virtual_env_options="--python=${python_exe}"
        fi
        virtual_env_options="${virtual_env_options:+$virtual_env_options }${verbosity}"
    elif "$python_exe" -c 'import venv' > /dev/null 2>&1
    then
        virtual_env="$python_exe -m venv"
        if ! "$python_exe" -c 'import ensurepip' > /dev/null 2>&1
        then
            virtual_env_options='--without-pip'
            install_pip='yes'
        fi
        msg 'Using venv from library' "${install_pip:+without pip}"
    else
        error_msg "'virtualenv' not available, and $python_exe has no 'venv' module."
        echo 'Please install virtualenv, or use the --with-python option to select a different Python executable.' >&2
        return 1
    fi

    # Create the virtual environment
    verbose_msg "${virtual_env} ${virtual_env_options} $venv_dir"
    if ! ${virtual_env} ${virtual_env_options} "$venv_dir"
    then
        error_msg "virtual environment creation failed." >&2
        return 1
    fi

    if [ -n "$install_pip" ]
    then
        if ( . "$activate_script" && "$venv_python" -m ensurepip --upgrade )
        then
            msg "Installed pip via ensurepip"
        else
            msg 'Bootstrapping pip from PyPA'
            if ! curl https://bootstrap.pypa.io/get-pip.py -s -S -o "$get_pip_py"
            then
                error_msg 'curl failed to download get-pip.py'
                return 1
            fi
            if ! ( . "$activate_script" && "$venv_python" "$get_pip_py" )
            then
                error_msg 'get_pip.py failed to install pip'
                return 1
            fi
        fi
    fi

    # Install required software
    if [ -e "$here/requirements.txt" ]
    then
        msg 'Installing requirements'
        if ! ( . "$activate_script" && \
                   "$venv_pip" install ${verbosity} -U pip && \
                   "$venv_pip" install ${verbosity} -r "$here/requirements.txt" )
        then
            error_msg "Installing requirements failed."
            return 1
        fi
    fi

    # Write out environment for validate script
    cat <<EOF > "$here/environment"
# Generated by setup.sh; do not edit
python_exe='$venv_dir/bin/python'
activate_script='$venv_dir/bin/activate'
EOF

    msg "Virtual environment successfully created in $venv_dir"
    return 0
}

upgrade_virtual_environment()
{
    msg "Upgrading virtual environment in $venv_dir"
    if ! ( . "$activate_script" && "$venv_python" -m venv --upgrade "$venv_dir" )
    then
        error_msg "Failed to upgrade virtual environment."
        return 1
    fi
    msg 'Upgrading requirements'
    if ! ( . "$activate_script" && "$venv_pip" install ${verbosity} --upgrade -r "$here/requirements.txt" )
    then
        error_msg "Failed to upgrade modules in the virtual environment."
        return 1
    fi

    msg "Virtual environment successfully upgraded in $venv_dir"
    return 0
}

if [ -d "$venv_dir" ] && [ -n "$do_reinstall" ]
then
    msg 'Deleting existing virtual environment'
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
