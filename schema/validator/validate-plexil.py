#! /usr/bin/env python3

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

import click
import os
import os.path as path
import pickle
import sys
import xml.etree.ElementTree as ElementTree
import xmlschema

defaultSchemaFile = path.join(path.dirname(path.dirname(path.realpath(__file__))),
                              'core-plexil.xsd')

@click.command()

@click.option('-s', '--silent',
              is_flag=True,
              help="Don't generate output, only set exit status")

@click.option('-v', '--verbose',
              is_flag=True,
              help="Generate extra output")

@click.option('--schema',
              type=click.Path(exists=True),
              default=defaultSchemaFile,
              help="Schema file to validate against")

@click.option('--cache',
              is_flag=True,
              help="Use schema cache")

@click.argument('infiles',
                type=click.Path(exists=True),
                nargs=-1)

# Main function
def validate(infiles, schema, silent, verbose, cache):
    """Validate XML files against a schema."""

    schm = loadSchema(schema, cache, verbose)
    if schm is None:
        sys.exit(1)

    if verbose:
        print('Validating against schema', schema)

    nFiles = len(infiles)
    nValid = 0
    nInvalid = 0
    nError = 0

    for infile in infiles:
        if silent:
            status = validateSilently(schm, infile)
        else:
            status = validateWithErrorMessages(schm, infile, verbose)
        if status > 0:
            nInvalid = nInvalid + 1
            if not silent:
                print(infile, 'failed validation with', status, 'errors')
        elif status < 0:
            nError = nError + 1
            if not silent:
                print(infile, 'could not be validated due to error')
        else:
            nValid = nValid + 1
            if verbose:
                print(infile, 'is valid')

    if nFiles > 1 and not silent:
        print(nFiles, 'files checked,', nValid, 'valid')
        if nInvalid > 0:
            print('  ', nInvalid, 'files invalid')
        if nError > 0:
            print('  ', nError, 'could not be validated')

    if nError > 0:
        sys.exit(2)
    elif nInvalid > 0:
        sys.exit(1)
    else:
        sys.exit(0)
        

# Load the named schema file and return it.
# Return None in event of error.
def loadSchema(schema, cache, verbose):
    name = path.basename(schema).replace('.xsd', '', 1)

    if cache:
        result = loadCachedSchema(name)
        if isinstance(result, xmlschema.validators.schema.XMLSchemaBase):
            if verbose:
                print('Loaded schema', schema, 'from cache')
            return result;

    try:
        result = xmlschema.XMLSchema11(schema, validation='lax')
        if verbose:
            print('Loaded schema', schema)

    except xmlschema.exceptions.XMLSchemaException as x:
        print('Error reading schema ', schema, ':\n ', str(x),
              sep='', file=sys.stderr)
        return None;

    except ElementTree.ParseError as p:
        print('XML parse error reading schema ', schema, ':\n ', str(p),
              sep='', file=sys.stderr)
        return None;

    except Exception as e:
        print('Error of type ', type(e).__name__, ' reading schema ', schema, ':\n ', str(e),
              sep='', file=sys.stderr)
        return None;

    if cache:
        cachename = cacheSchema(result, name)
        if verbose and cachename is not None:
            print('Schema cached to', cachename)

    return result

# Return the file path if successful, None if not
def cacheSchema(schm, name):
    dumpname = schemaCachePath(name)

    dumpdir = path.dirname(dumpname)
    if not path.isdir(dumpdir):
        try:
            os.makedirs(dumpdir, mode=0o755, exist_ok=True)
        except Exception as e:
            print('Exception of type ', type(e).__name__,
                  ' creating directory ', dumpdir, ':\n ', str(e),
                  sep='', file=sys.stderr)
            return None

    try:
        with open(dumpname, mode='w+b') as dumpfile:
            pickle.dump(schm, dumpfile)
            return dumpname

    except OSError as o:
        print('OS error writing schema to ', dumpname, ':\n ', str(o),
              sep='', file=sys.stderr)
        return None

    except Exception as e:
        print('Exception of type ', type(e).__name__,
              ' writing schema to ', dumpname, ':\n ', str(e),
              sep='', file=sys.stderr)
        close(dumpfile)
        return None

# Return the cached schema for the name, if it exists.
def loadCachedSchema(name):
    loadpath = schemaCachePath(name)
    if not path.exists(loadpath) or not path.isfile(loadpath):
        return None

    try:
        with open(loadpath, mode='r+b') as loadfile:
            result = pickle.load(loadfile)

    except OSError as o:
        print('OS error reading schema from ', loadname, ':\n ', str(o),
              sep='', file=sys.stderr)
        return None

    except Exception as e:
        print('Exception of type ', type(e).__name__,
              ' reading schema from ', loadname, ':\n ', str(e),
              sep='', file=sys.stderr)
        return None

    if isinstance(result, xmlschema.validators.schema.XMLSchemaBase):
        return result
    else:
        return None
    
def schemaCacheDir():
    return path.join(path.dirname(__file__), 'schema_cache')
    
def schemaCachePath(name):
    return path.join(schemaCacheDir(), "{0}.xsc".format(name))


# Validate the file, returning 0 if valid,
# 1 if invalid, or -1 if an exception occurred
def validateSilently(schm, infile):
    try:
        if schm.is_valid(infile):
            return 0
        else:
            return 1

    except ElementTree.ParseError as p:
        print(infile, 'could not be parsed as XML')
        return -1

    except Exception as e:
        print('Unknown error while validating ', infile, ':\n ', str(e),
              sep='', file=sys.stderr)
        return -1

# Validate the file, printing each validation error
# Return the number of errors, or -1 if some other exception occurred
def validateWithErrorMessages(schm, infile, verbose):
    if verbose:
        print('Validating', infile)

    errorCount = 0

    try:
        for err in schm.iter_errors(infile):
            errorCount = errorCount + 1
            if verbose:
                print('  ', str(err))
            elif err.path is not None:
                print('  ', err.message, '\n  at', err.path)
            else:
                print('  ', err.message)
        return errorCount

    except ElementTree.ParseError as p:
        print('XML parse error for ', infile, ':\n ', str(p),
              sep='', file=sys.stderr)
        return -1

    except Exception as e:
        print('Unknown error while validating ', infile, ':\n ', str(e),
              sep='', file=sys.stderr)
        return -1
        
if __name__ == '__main__':
    validate()
