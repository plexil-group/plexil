#! /usr/bin/env python3

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

import os
from os import path
import pickle
import sys
import traceback # debugging help
from urllib.parse import urlsplit
from xml.etree import ElementTree
import click
import xmlschema
from xmlschema.resources import is_local_url, normalize_url

default_schema_file = path.join(path.dirname(path.dirname(path.realpath(__file__))),
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
              default=default_schema_file,
              help="Schema file to validate against")

@click.option('--cache',
              is_flag=True,
              help="Use schema cache, if schema is a local file")

# Maybe later
# @click.option('--forceCache',
#               is_flag=True,
#               help="Use cache even if schema is not a local file")

@click.argument('infiles',
                type=click.Path(exists=True),
                nargs=-1)

# Main function
# pylint disable=too-many-branches
def validate(infiles, schema, silent, verbose, cache):
    """Validate XML files against a schema."""

    schm = load_schema(schema, cache, verbose)
    if schm is None:
        sys.exit(1)

    if verbose:
        print('Validating against schema', schema)

    n_files = len(infiles)
    n_valid = 0
    n_invalid = 0
    n_error = 0

    for infile in infiles:
        if silent:
            status = validate_silently(schm, infile)
        else:
            status = validate_with_error_messages(schm, infile, verbose)
        if status > 0:
            n_invalid = n_invalid + 1
            if not silent:
                print(infile, 'failed validation with', status, 'errors')
        elif status < 0:
            n_error = n_error + 1
            if not silent:
                print(infile, 'could not be validated due to error')
        else:
            n_valid = n_valid + 1
            if verbose:
                print(infile, 'is valid')

    if n_files > 1 and not silent:
        print(n_files, 'files checked,', n_valid, 'valid')
        if n_invalid > 0:
            print('  ', n_invalid, 'files invalid')
        if n_error > 0:
            print('  ', n_error, 'could not be validated')

    if n_error > 0:
        sys.exit(2)
    if n_invalid > 0:
        sys.exit(1)
    sys.exit(0)


# Load the named schema file and return it.
# Return None in event of error.
def load_schema(schema_url, cache, verbose):

    """Load an XML schema from the given URL, or from a local cache file."""

    if cache and schema_is_cacheable(schema_url):
        schema_path = schema_local_path(schema_url)
        if not schema_newer_than_cache(schema_path):
            cache_path = schema_cache_path(schema_name_from_url(schema_url))
            result = load_cached_schema(cache_path)
            if result:
                if verbose:
                    print('Loaded cached schema from', cache_path)
                return result

    # Continue here if loading from cache failed somehow
    result = load_schema_from_url(schema_url, verbose)
    if result and cache and schema_is_cacheable(schema_url):
        cachename = cache_schema(result, schema_name_from_url(schema_url))
        if verbose and cachename:
            print('Schema', schema_url, 'cached to', cachename)

    return result


def load_schema_from_url(schema_url, verbose):

    """Load an XML Schema 1.1 schema instance from the given URL.
       Returns None in the event of failure."""

    try:
        result = xmlschema.XMLSchema11(schema_url, validation='lax')
        if isinstance(result, xmlschema.validators.schemas.XMLSchemaBase):
            if verbose:
                print('Loaded schema', schema_url)
            return result
        print('Schema URL', schema_url, 'does not contain a valid XML 1.1 schema',
              file=sys.stderr)
        return None

    except xmlschema.exceptions.XMLSchemaException as schema_exc:
        print('Error reading schema ', schema_url, ':\n ', str(schema_exc),
              sep='', file=sys.stderr)
        return None

    except ElementTree.ParseError as parse_err:
        print('XML parse error reading schema ', schema_url, ':\n ', str(parse_err),
              sep='', file=sys.stderr)
        return None

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Error of type ', type(unexpected_exc).__name__,
              ' reading schema ', schema_url, ':\n ',
              str(unexpected_exc),
              sep='', file=sys.stderr)
        return None


def cache_schema(schema, name):

    """Cache the schema instance under the given name.
       Returns the pathname of the cache file if successful."""

    dumpname = schema_cache_path(name)
    dumpdir = path.dirname(dumpname)
    if not path.isdir(dumpdir):
        try:
            os.makedirs(dumpdir, mode=0o755, exist_ok=True)
        # pylint disable=broad-exception-caught
        except Exception as unexpected_exc:
            print('Exception of type ', type(unexpected_exc).__name__,
                  ' creating directory ', dumpdir, ':\n ', str(unexpected_exc),
                  sep='', file=sys.stderr)
            return None

    try:
        with open(dumpname, mode='wb') as dumpfile:
            pickle.dump(schema, dumpfile, pickle.HIGHEST_PROTOCOL)
            return dumpname

    except OSError as os_err:
        print('OS error writing schema ', schema.url, '  to ', dumpname, ':\n ', str(os_err),
              sep='', file=sys.stderr)
        return None

    # pylint disable=broad-exception-caught
    except Exception as unexpected:
        print('Unexpected exception ', type(unexpected).__name__,
              ' writing schema ', schema.url, ' to ', dumpname, ':\n ', str(unexpected),
              sep='', file=sys.stderr)
        return None


def load_cached_schema(loadpath):

    """Load a cached schema from the named file.
       Returns None in the event of failure."""

    if not loadpath or not path.isfile(loadpath):
        return None

    try:
        with open(loadpath, mode='rb') as loadfile:
            result = pickle.load(loadfile)

    except OSError as os_err:
        print('OS error reading schema from cache file ', loadpath, ':\n ', str(os_err),
              sep='', file=sys.stderr)
        return None

    except pickle.UnpicklingError as unpickle_err:
        print('Unpickling error reading schema from cache file ', loadpath, ':\n ',
              str(unpickle_err),
              sep='', file=sys.stderr)
        unpickle_exc_info = sys.exc_info()
        if unpickle_exc_info[2]: # we have a traceback
            traceback.print_exception(*unpickle_exc_info, file=sys.stderr)
        return None

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Exception of type ', type(unexpected_exc).__name__,
              ' reading schema from cache file ', loadpath, ':\n ', str(unexpected_exc),
              sep='', file=sys.stderr)
        return None

    if isinstance(result, xmlschema.validators.schemas.XMLSchemaBase):
        return result

    print('Warning: cache file ', loadpath, ' did not contain a valid schema',
          sep='', file=sys.stderr)
    return None

def schema_name_from_url(schema_url):

    """Get the schema name from a schema URL."""

    try:
        parsed_url = urlsplit(schema_url)
        return path.splitext(path.basename(parsed_url.path))[0]

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Error getting schema short name from ', schema_url, ':\n ',
              str(unexpected_exc),
              sep='', file=sys.stderr)
        return None

# Only cache schema from local files for now
def schema_is_cacheable(schema_url):

    """Determine whether schema can be cached locally based on its URL."""

    return is_local_url(schema_url) and schema_name_from_url(schema_url)


# Only called if schema_is_cacheable returns true and schema_file exists
def schema_newer_than_cache(schema_file):

    """Returns True if schema is newer than or as old as its local cache file,
       False if older. Returns True on error."""

    cache_path = schema_cache_path(schema_name_from_url(schema_file))
    if not path.isfile(cache_path):
        return True
    try:
        cache_date = path.getmtime(cache_path)

    except OSError as os_err:
        print('OS error getting modified date from ', cache_path, ':\n ', str(os_err),
              sep='', file=sys.stderr)
        return True

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Error getting modified date from ', cache_path, ':\n ', str(unexpected_exc),
              sep='', file=sys.stderr)
        return True

    # We know schema_file exists
    try:
        return path.getmtime(schema_file) >= cache_date

    except OSError as os_err:
        print('OS error getting modified date from ', schema_file, ':\n ', str(os_err),
              sep='', file=sys.stderr)
        return True

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Error getting modified date from ', schema_file, ':\n ', str(unexpected_exc),
              sep='', file=sys.stderr)
        return True


def schema_local_path(schema_url):

    """Returns the path of the local file for this schema URL"""

    if not is_local_url(schema_url):
        return None

    if path.isfile(schema_url):
        return schema_url

    normalized_path = urlsplit(normalize_url(schema_url)).path
    if path.isfile(normalized_path):
        return normalized_path

    return None


# FIXME: allow caching in locations other than this directory
def schema_cache_dir():

    """Get the schema cache directory path."""

    return path.join(path.dirname(__file__), 'schema_cache')


def schema_cache_path(name):

    """Returns the file name for the schema cache, based on the schema name"""

    return path.join(schema_cache_dir(), "{0}.xsc".format(name))


def validate_silently(schm, infile):

    """Validate the file, returning 0 if valid,
       1 if invalid, or -1 if an error occurred."""

    try:
        if schm.is_valid(infile):
            return 0
        return 1

    except ElementTree.ParseError as parse_err:
        print(infile, 'could not be parsed as XML:\n ', str(parse_err),
              file=sys.stderr)
        return -1

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Unknown error while validating ', infile, ':\n ', str(unexpected_exc),
              sep='', file=sys.stderr)
        return -1


def validate_with_error_messages(schm, infile, verbose):

    """Validate the file, printing each validation error.
       Return the number of errors, or -1 if some error occurred."""

    if verbose:
        print('Validating', infile)

    error_count = 0

    try:
        for err in schm.iter_errors(infile):
            error_count = error_count + 1
            if verbose:
                print('  ', str(err))
            elif err.path is not None:
                print(' ', err.message, '\n ', err.reason, '\n  at', err.path)
            else:
                print('  ', err.message)
        return error_count

    except ElementTree.ParseError as parse_err:
        print('XML parse error for ', infile, ':\n ', str(parse_err),
              sep='', file=sys.stderr)
        return -1

    # pylint disable=broad-exception-caught
    except Exception as unexpected_exc:
        print('Unknown error while validating ', infile, ':\n ', str(unexpected_exc),
              sep='', file=sys.stderr)
        return -1


if __name__ == '__main__':
    # click handles the parameter passing here
    # pylint disable=no-value-for-parameter
    validate()
