# PLEXIL decompiler

The decompiler is run in a similar fashion to the summarize-plexil script. The script is named `scripts/decompile-plexil`, which reads a .plx file and produced .ple that would compile to this output.

Tests for the decompiler are inside the `tests` folder. The `decompiler-single-file` test script verifies that the decompiler can decompile a .plx file, yielding a program whose compiled form matches that of the original file. The `decompiler-test` script runs the previous script on most plx files inside the top-level Plexil directory.
