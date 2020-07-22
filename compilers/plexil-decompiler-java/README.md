## Plexil Decompiler Java

### About
This is a decompiler built to convert PLEXIL plans that have been converted to XML (.plx) back into PLEXIL plans (.ple).

Currently, this project exists at a proof-of-concept level of functionality, and can decompile programs built using basic assignment and arithmetic operations, as well as the `for` and `while` control structures.

### Running the Decompiler
As no platform-friendly executable option exists yet, the way to run this decompiler is to load it into an IDE with Java compilation capabilities, and to navigate to the `Decompiler.java` file in `src/plexiljava.main/`. In that file, you can change the `INFILE` constant to the name of the file you wish to decompile that exists in the `plexil-decompiler-java` folder, as well as the `OUTFILE` constant to the name of the file you wish to decompile to. Upon execution, the program will generate a corresponding file in the `plexil-decompiler-java`, containing a PLEXIL plan that is functionally identical\* to the original compilation source.

### Included Tools
Another point of interest is the `XMLNavigator.java` file, also found in the `src/plexiljava.main/` subdirectory. This program has a `FILENAME` constant which can be set to any compiled PLEXIL plan's filename, and when run, will allow the user to navigate the corresponding XML structure as though it were a unix directory.

Currently, this navigator supports the following commands:
* `ls`     - List out all child nodes, numbered by order of occurence
* `cd <#>` - Change focus to the child node specified by the given number
* `cd ..`  - Go back to parent Node
* `attr`   - List out the XML attributes of the current Node
* `qual`   - List out the qualities\*\* of the current Node



\* - *here meaning a PLEXIL plan that would compile back into the decompiled XML file*
\*\* - *a quality is a child element that only has one `#text` child node, and is treated like an attribute*
