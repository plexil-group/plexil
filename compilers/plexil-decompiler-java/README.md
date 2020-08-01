## Plexil Decompiler Java

### About
This is a decompiler built to convert PLEXIL plans that have been converted to XML (.plx) back into PLEXIL plans (.ple).

Currently, this project exists at a proof-of-concept level of functionality, and can decompile programs built using basic assignment and arithmetic operations, as well as the `for` and `while` control structures.

### Running the Decompiler
To run the decompiler, navigate to the root directory (`plexil-decompiler-java`) from the commandline, and run the jar file in the `bin` folder as follows: `java -jar bin/deplexil.jar FILE_TO_DECOMPILE OUTPUT_FILE`
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
