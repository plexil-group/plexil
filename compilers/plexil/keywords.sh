#! /bin/sh -e
# Generate list of keywords from contents of ANTLR tokens file
# and stream them to stdout

usage()
{
    echo "Usage: $(basename "$0") <infile> <outfile>"
}

if [ $# -lt 2 ]
then
    echo "$(basename "$0"): Error: required parameter(s) missing" >&2
    usage >&2
    exit 2
fi

cat "$1" | grep "^'[A-Za-z_]" | sed -e "s/^'\([A-Za-z_][A-Za-z0-9_]*\)'=.*$/\1/g" > "$2" | sort
