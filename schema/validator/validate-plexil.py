#! /usr/bin/env python3

import click
import os.path as path
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
@click.argument('infile',
                type=click.Path(exists=True))

def validate(infile, schema, silent, verbose):
    """Validate an XML file against a schema."""
    if verbose:
        print('Validating', infile, 'against schema', schema)

    try:
        schm = xmlschema.XMLSchema11(schema)
    except xmlschema.exceptions.XMLSchemaException as x:
        print('Error reading schema ', schema, ':\n ', str(x),
              sep='', file=sys.stderr)
        sys.exit(1)
    except ElementTree.ParseError as p:
        print('XML parse error reading schema ', schema, ':\n ', str(p),
              sep='', file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print('Error of type ', type(e).__name__, ' reading schema ', schema, ':\n ', str(e),
              sep='', file=sys.stderr)
        sys.exit(2)

    try:
        if silent:
            sys.exit(not schm.is_valid(infile))
        else:
            schm.validate(infile)
            print(infile, 'validates.')
            sys.exit(0)
    except xmlschema.exceptions.XMLSchemaException as x:
        if not silent:
            print('XML validation error for ', infile, ':\n ', str(x),
                  sep='', file=sys.stderr)
            print(infile, 'failed validation.')
        sys.exit(1)
    except ElementTree.ParseError as p:
        print('XML parse error for ', infile, ':\n ', str(p),
              sep='', file=sys.stderr)
        if not silent:
            print(infile, 'failed validation.')
        sys.exit(1)
    except Exception as e:
        print('Unknown error while validating ', infile, ':\n ', str(e),
              sep='', file=sys.stderr)
        sys.exit(2)

if __name__ == '__main__':
    validate()
