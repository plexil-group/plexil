This directory contains an XML Schema 1.1 validator, implemented as a
Python application.  It is based on `xmlschema`:

[Source code](https://github.com/sissaschool/xmlschema "Github repository") 
[Documentation](https://xmlschema.readthedocs.io/en/latest/ "Documentation at readthedocs.io")

`xmlschema` is the easiest-to-use XML Schema 1.1 validator this author
is aware of.  It is available through the Python Package Index.

## Requirements

- Python 3.5 or newer
- Either 'virtualenv' or 'venv' module installed
- Internet access to pypi.org and pypa.org
- Writable directory with 30 MB available for Python virtual environment

## Installation

Run setup.sh in this directory.

## Usage

    validate [options] <infile> ...

Validates XML files against an XML Schema 1.1 schema.  Writes error
messages to stdout.

Options:
  -s, --silent    Don't generate output, only set exit status
  -v, --verbose   Generate extra output
  --schema PATH   Schema file to validate against
  --cache         Use schema cache, if schema is a local file
  --help          Show usage message and exit.

Return status:
* 0 if all files were valid
* 1 if one or more failed validation
* 2 if some other error occurred.
