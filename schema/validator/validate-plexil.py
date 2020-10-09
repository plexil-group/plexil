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
import traceback # debugging help
from urllib.parse import urlsplit
import xml.etree.ElementTree as ElementTree
import xmlschema
from xmlschema.resources import is_local_url, normalize_url, url_path_is_file

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
              help="Use cache if schema is a local file")

# Maybe later
# @click.option('--forceCache',
#               is_flag=True,
#               help="Use cache even if schema is not a local file")

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
def loadSchema(schemaUrl, cache, verbose):

    """Load an XML schema from the given URL, or from a local cache file."""

    if cache and schemaIsCacheable(schemaUrl):
        schemaPath = schemaLocalPath(schemaUrl)
        if not schemaNewerThanCache(schemaPath):
            cachePath = schemaCachePath(schemaNameFromUrl(schemaUrl))            
            result = loadCachedSchema(cachePath)
            if result:
                if verbose:
                    print('Loaded cached schema from', cachePath)
                return result

    # Continue here if loading from cache failed somehow
    result = loadSchemaFromUrl(schemaUrl, verbose)
    if result and cache and schemaIsCacheable(schemaUrl):
        cachename = cacheSchema(result, schemaNameFromUrl(schemaUrl))
        if verbose and cachename:
            print('Schema', schemaUrl, 'cached to', cachename)

    return result


def loadSchemaFromUrl(schemaUrl, verbose):

    """Load an XML Schema 1.1 schema instance from the given URL.
       Returns None in the event of failure."""

    try:
        result = xmlschema.XMLSchema11(schemaUrl, validation='lax')
        if isinstance(result, xmlschema.validators.schema.XMLSchemaBase):
            if verbose:
                print('Loaded schema', schemaUrl)
            return result;
        else:
            print('Schema URL', schemaUrl, 'does not contain a valid XML 1.1 schema',
                  file=syst.stderr)
            return None;

    except xmlschema.exceptions.XMLSchemaException as x:
        print('Error reading schema ', schemaUrl, ':\n ', str(x),
              sep='', file=sys.stderr)
        return None;

    except ElementTree.ParseError as p:
        print('XML parse error reading schema ', schemaUrl, ':\n ', str(p),
              sep='', file=sys.stderr)
        return None;

    except Exception as e:
        print('Error of type ', type(e).__name__, ' reading schema ', schemaUrl, ':\n ', str(e),
              sep='', file=sys.stderr)
        return None;


def cacheSchema(schema, name):

    """Cache the schema instance under the given name.
       Returns the pathname of the cache file if successful."""

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
        with open(dumpname, mode='wb') as dumpfile:
            pickle.dump(schema, dumpfile, pickle.HIGHEST_PROTOCOL)
            return dumpname

    except OSError as o:
        print('OS error writing schema ', schema.url, '  to ', dumpname, ':\n ', str(o),
              sep='', file=sys.stderr)
        return None

    except Exception as e:
        print('Exception of type ', type(e).__name__,
              ' writing schema ', schema.url, ' to ', dumpname, ':\n ', str(e),
              sep='', file=sys.stderr)
        close(dumpfile)
        return None


def loadCachedSchema(loadpath):

    """Load a cached schema from the named file. 
       Returns None in the event of failure."""

    if not loadpath or not path.isfile(loadpath):
        return None

    try:
        with open(loadpath, mode='rb') as loadfile:
            result = pickle.load(loadfile)

    except OSError as o:
        print('OS error reading schema from cache file ', loadpath, ':\n ', str(o),
              sep='', file=sys.stderr)
        return None

    except pickle.UnpicklingError as u:
        print('Unpickling error reading schema from cache file ', loadpath, ':\n ', str(u),
              sep='', file=sys.stderr)
        excInfo = sys.exc_info()
        if excInfo[2]: # we have a traceback
            traceback.print_exception(excInfo[0], excInfo[1], excInfo[2], file=sys.stderr)
        return None

    except Exception as e:
        print('Exception of type ', type(e).__name__,
              ' reading schema from cache file ', loadpath, ':\n ', str(e),
              sep='', file=sys.stderr)
        return None

    if isinstance(result, xmlschema.validators.schema.XMLSchemaBase):
        return result

    else:
        print('Warning: cache file ', loadpath, ' did not contain a valid schema',
              sep='', file=sys.stderr)
        return None

def schemaNameFromUrl(schemaUrl):
    try:
        parsedUrl = urlsplit(schemaUrl);
        return path.splitext(path.basename(parsedUrl.path))[0]

    except Exception as e:
        print('Error getting schema short name from ', schemaUrl, ':\n ', str(e),
              sep='', file=sys.stderr)
        return None

# Only cache schema from local files for now
def schemaIsCacheable(schemaUrl):

    """Determine whether schema can be cached locally based on its URL."""

    return url_path_is_file(schemaUrl) and schemaNameFromUrl(schemaUrl)


# Only called if schemaIsCacheable returns true and schemaFile exists
def schemaNewerThanCache(schemaFile):

    """Returns True if schema is newer than or as old as its local cache file,
       False if older. Returns True on error."""

    cachePath = schemaCachePath(schemaNameFromUrl(schemaFile))
    if not path.isfile(cachePath):
        return True
    try:
        cacheDate = path.getmtime(cachePath)

    except OSError as o:
        print('OS error getting modified date from ', cachePath, ':\n ', str(o),
              sep='', file=sys.stderr)
        return True

    except Exception as e:
        print('Error getting modified date from ', cachePath, ':\n ', str(e),
              sep='', file=sys.stderr)
        return True

    # We know schemaFile exists
    try:
        return path.getmtime(schemaFile) >= cacheDate

    except OSError as o:
        print('OS error getting modified date from ', schemaFile, ':\n ', str(o),
              sep='', file=sys.stderr)
        return True

    except Exception as e:
        print('Error getting modified date from ', schemaFile, ':\n ', str(e),
              sep='', file=sys.stderr)
        return True


def schemaLocalPath(schemaUrl):
    
    """Returns the path of the local file for this schema URL"""

    if not is_local_url(schemaUrl):
        return None

    if path.isfile(schemaUrl):
        return schemaUrl;

    normalizedPath = urlsplit(normalize_url(schemaUrl)).path
    if path.isfile(normalizedPath):
        return normalizedPath

    return None
    

# FIXME: allow caching in locations other than this directory
def schemaCacheDir():
    return path.join(path.dirname(__file__), 'schema_cache')



def schemaCachePath(name):

    """Returns the file name for the schema cache, based on the schema name"""

    return path.join(schemaCacheDir(), "{0}.xsc".format(name))


def validateSilently(schm, infile):

    """Validate the file, returning 0 if valid,
       1 if invalid, or -1 if an error occurred."""

    try:
        if schm.is_valid(infile):
            return 0
        else:
            return 1

    except ElementTree.ParseError as p:
        print(infile, 'could not be parsed as XML', file=sys.stderr)
        return -1

    except Exception as e:
        print('Unknown error while validating ', infile, ':\n ', str(e),
              sep='', file=sys.stderr)
        return -1

    
def validateWithErrorMessages(schm, infile, verbose):

    """Validate the file, printing each validation error.
       Return the number of errors, or -1 if some error occurred."""

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
